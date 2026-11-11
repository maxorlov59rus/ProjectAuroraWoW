#include "Auth/Srp6.h"

#include "Misc/SecureHash.h"
#include "Math/UnrealMathUtility.h"

namespace Aurora::Srp6
{
	namespace
	{
		using FBigInt = TArray<uint8>;

		/** N из AzerothCore, 32 байта LE. */
		const uint8 NBytes[32] = {
			0xB7, 0x9B, 0x3E, 0x2A, 0x87, 0x82, 0x3C, 0xAB,
			0x8F, 0x5E, 0xBF, 0xBF, 0x8E, 0xB1, 0x01, 0x08,
			0x53, 0x50, 0x06, 0x29, 0x8B, 0x5B, 0xAD, 0xBD,
			0x5B, 0x53, 0xE1, 0x89, 0x5E, 0x64, 0x4B, 0x89
		};

		const uint8 G = 7;

		FBigInt Trim(FBigInt In)
		{
			int32 Len = In.Num();
			while (Len > 0 && In[Len - 1] == 0)
			{
				--Len;
			}
			In.SetNum(Len);
			return In;
		}

		int32 Compare(const FBigInt& A, const FBigInt& B)
		{
			const FBigInt TA = Trim(A);
			const FBigInt TB = Trim(B);
			if (TA.Num() != TB.Num())
			{
				return TA.Num() < TB.Num() ? -1 : 1;
			}
			for (int32 i = TA.Num() - 1; i >= 0; --i)
			{
				if (TA[i] != TB[i])
				{
					return TA[i] < TB[i] ? -1 : 1;
				}
			}
			return 0;
		}

		/** A - B, A >= B. */
		FBigInt Sub(const FBigInt& A, const FBigInt& B)
		{
			FBigInt Out;
			Out.SetNumZeroed(A.Num());
			int32 Borrow = 0;
			for (int32 i = 0; i < A.Num(); ++i)
			{
				int32 Diff = static_cast<int32>(A[i]) - Borrow - (i < B.Num() ? B[i] : 0);
				if (Diff < 0)
				{
					Diff += 256;
					Borrow = 1;
				}
				else
				{
					Borrow = 0;
				}
				Out[i] = static_cast<uint8>(Diff);
			}
			return Trim(Out);
		}

		FBigInt Mul(const FBigInt& A, const FBigInt& B)
		{
			FBigInt Out;
			Out.SetNumZeroed(A.Num() + B.Num() + 1);
			for (int32 i = 0; i < A.Num(); ++i)
			{
				if (A[i] == 0)
				{
					continue;
				}
				int32 Carry = 0;
				for (int32 j = 0; j < B.Num(); ++j)
				{
					const int32 Cur = Out[i + j] + A[i] * B[j] + Carry;
					Out[i + j] = static_cast<uint8>(Cur & 0xFF);
					Carry = Cur >> 8;
				}
				int32 k = i + B.Num();
				while (Carry > 0 && k < Out.Num())
				{
					const int32 Cur = Out[k] + Carry;
					Out[k] = static_cast<uint8>(Cur & 0xFF);
					Carry = Cur >> 8;
					++k;
				}
			}
			return Trim(Out);
		}

		bool BitAt(const FBigInt& Value, int32 Bit)
		{
			const int32 Byte = Bit >> 3;
			if (Byte >= Value.Num())
			{
				return false;
			}
			return ((Value[Byte] >> (Bit & 7)) & 1) != 0;
		}

		FBigInt Mod(const FBigInt& Value, const FBigInt& Divider)
		{
			FBigInt Remainder;
			// Битовое деление: сдвигаем остаток влево и добавляем бит.
			const int32 TopBit = Value.Num() * 8 - 1;
			for (int32 Bit = TopBit; Bit >= 0; --Bit)
			{
				// Rem = Rem << 1
				uint8 Carry = 0;
				for (int32 i = 0; i < Remainder.Num(); ++i)
				{
					const uint8 Next = static_cast<uint8>(Remainder[i] >> 7);
					Remainder[i] = static_cast<uint8>((Remainder[i] << 1) | Carry);
					Carry = Next;
				}
				if (Carry)
				{
					Remainder.Add(Carry);
				}

				// Rem = Rem | bit
				if (BitAt(Value, Bit))
				{
					if (Remainder.Num() == 0)
					{
						Remainder.Add(1);
					}
					else
					{
						Remainder[0] |= 1;
					}
				}

				if (Compare(Remainder, Divider) >= 0)
				{
					Remainder = Sub(Remainder, Divider);
				}
			}
			return Trim(Remainder);
		}

		FBigInt MulMod(const FBigInt& A, const FBigInt& B, const FBigInt& N)
		{
			return Mod(Mul(A, B), N);
		}

		FBigInt Add(const FBigInt& A, const FBigInt& B)
		{
			const int32 Len = FMath::Max(A.Num(), B.Num());
			FBigInt Out;
			Out.SetNumZeroed(Len + 1);
			int32 Carry = 0;
			for (int32 i = 0; i < Len; ++i)
			{
				const int32 Cur = (i < A.Num() ? A[i] : 0) + (i < B.Num() ? B[i] : 0) + Carry;
				Out[i] = static_cast<uint8>(Cur & 0xFF);
				Carry = Cur >> 8;
			}
			Out[Len] = static_cast<uint8>(Carry);
			return Trim(Out);
		}

		FBigInt ModAdd(const FBigInt& A, const FBigInt& B, const FBigInt& N)
		{
			FBigInt Sum = Add(A, B);
			if (Compare(Sum, N) >= 0)
			{
				Sum = Sub(Sum, N);
			}
			return Sum;
		}

		/** Сообщение -> SHA1, 20 байт. */
		void Sha1(const uint8* Data, int32 Len, uint8* Out20)
		{
			FSHA1::HashBuffer(Data, static_cast<uint64>(Len), Out20);
		}

		void Sha1(const FBigInt& Data, FBigInt& Out)
		{
			uint8 Digest[20];
			Sha1(Data.GetData(), Data.Num(), Digest);
			Out.Reset();
			Out.Append(Digest, 20);
		}

		/** Приводит значение к ровно 32 байтам LE. */
		FBigInt Pad32(FBigInt Value)
		{
			Value = Trim(Value);
			Value.SetNumZeroed(32);
			return Value;
		}
	}

	void ComputeUserPasswordHash(const FString& UsernameUpper, const FString& Password, TArray<uint8>& OutHash)
	{
		const FString UserPass = UsernameUpper + TEXT(":") + Password;

		// Протокол берёт внутренний хеш от ASCII-представления, приведённого
		// к верхнему регистру; разделитель — литеральное двоеточие.
		FTCHARToUTF8 Utf8(*UserPass);
		uint8 Digest[20];
		Sha1(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length(), Digest);

		OutHash.Reset();
		OutHash.Append(Digest, 20);
	}

	void ComputeX(const TArray<uint8>& Salt, const TArray<uint8>& UserPasswordHash, TArray<uint8>& OutX)
	{
		FBigInt Message;
		Message.Append(Salt.GetData(), FMath::Min(Salt.Num(), 32));
		while (Message.Num() < 32)
		{
			Message.Add(0);
		}
		Message.Append(UserPasswordHash.GetData(), UserPasswordHash.Num());

		Sha1(Message, OutX);
	}

	void ComputeVerifier(const TArray<uint8>& Salt, const TArray<uint8>& UserPasswordHash, TArray<uint8>& OutVerifier)
	{
		FBigInt X;
		ComputeX(Salt, UserPasswordHash, X);

		FBigInt N;
		N.Append(NBytes, 32);

		FBigInt GBig;
		GBig.Add(G);

		// v = g^x mod N
		FBigInt Result;
		Result.Add(1);
		FBigInt Base = GBig;
		for (int32 Bit = 0; Bit < X.Num() * 8; ++Bit)
		{
			if ((X[Bit >> 3] >> (Bit & 7)) & 1)
			{
				Result = MulMod(Result, Base, N);
			}
			Base = MulMod(Base, Base, N);
		}

		OutVerifier = Pad32(Result);
	}

	void GeneratePublicA(TArray<uint8>& OutA, TArray<uint8>& OutPrivateA)
	{
		OutPrivateA.Reset();
		OutPrivateA.SetNumZeroed(19);
		for (int32 i = 0; i < 19; ++i)
		{
			OutPrivateA[i] = static_cast<uint8>(FMath::RandRange(0, 255));
		}
		// Верхний байт не должен быть нулевым, иначе уменьшается энтропия.
		if (OutPrivateA[18] == 0)
		{
			OutPrivateA[18] = 1;
		}

		FBigInt N;
		N.Append(NBytes, 32);

		FBigInt GBig;
		GBig.Add(G);

		FBigInt Result;
		Result.Add(1);
		FBigInt Base = GBig;
		for (int32 Bit = 0; Bit < OutPrivateA.Num() * 8; ++Bit)
		{
			if ((OutPrivateA[Bit >> 3] >> (Bit & 7)) & 1)
			{
				Result = MulMod(Result, Base, N);
			}
			Base = MulMod(Base, Base, N);
		}

		OutA = Pad32(Result);
	}

	void ComputeU(const TArray<uint8>& A, const TArray<uint8>& B, TArray<uint8>& OutU)
	{
		FBigInt Message;
		Message.Append(Pad32(A));
		Message.Append(Pad32(B));
		Sha1(Message, OutU);
	}

	void ComputeS(
		const TArray<uint8>& B,
		const TArray<uint8>& PrivateA,
		const TArray<uint8>& U,
		const TArray<uint8>& X,
		TArray<uint8>& OutS)
	{
		FBigInt N;
		N.Append(NBytes, 32);

		// k = 3 (фиксированная константа AzerothCore)
		const uint8 K = 3;

		// S = (B - k * g^x)^(a + u*x) mod N
		FBigInt GBig;
		GBig.Add(G);
		FBigInt Gx;
		Gx.Add(1);
		FBigInt Base = GBig;
		for (int32 Bit = 0; Bit < X.Num() * 8; ++Bit)
		{
			if ((X[Bit >> 3] >> (Bit & 7)) & 1)
			{
				Gx = MulMod(Gx, Base, N);
			}
			Base = MulMod(Base, Base, N);
		}

		FBigInt Kgx;
		Kgx.Add(K);
		Kgx = MulMod(Kgx, Gx, N);

		FBigInt Bv = Pad32(B);
		FBigInt Diff;
		if (Compare(Bv, Kgx) >= 0)
		{
			Diff = Sub(Bv, Kgx);
		}
		else
		{
			Diff = Sub(Add(Bv, N), Kgx);
		}

		// Exp = a + u*x
		FBigInt Ux = Mul(U, X);
		FBigInt Exp = Add(PrivateA, Ux);

		FBigInt Result;
		Result.Add(1);
		FBigInt B2 = Mod(Diff, N);
		for (int32 Bit = 0; Bit < Exp.Num() * 8; ++Bit)
		{
			if ((Exp[Bit >> 3] >> (Bit & 7)) & 1)
			{
				Result = MulMod(Result, B2, N);
			}
			B2 = MulMod(B2, B2, N);
		}

		OutS = Pad32(Result);
	}

	void ComputeM1(
		const TArray<uint8>& Username,
		const TArray<uint8>& Salt,
		const TArray<uint8>& A,
		const TArray<uint8>& B,
		const TArray<uint8>& S,
		TArray<uint8>& OutM1)
	{
		FBigInt N;
		N.Append(NBytes, 32);
		FBigInt GBig;
		GBig.Add(G);

		FBigInt HN;
		Sha1(N, HN);
		FBigInt HG;
		Sha1(GBig, HG);

		FBigInt XorHN;
		XorHN.SetNumZeroed(20);
		for (int32 i = 0; i < 20; ++i)
		{
			XorHN[i] = static_cast<uint8>(HN[i] ^ HG[i]);
		}

		FBigInt HUsername;
		FBigInt NameBytes;
		NameBytes.Append(Username.GetData(), Username.Num());
		Sha1(NameBytes, HUsername);

		FBigInt Message;
		Message.Append(XorHN);
		Message.Append(HUsername);
		Message.Append(Salt.GetData(), FMath::Min(Salt.Num(), 32));
		Message.Append(Pad32(A));
		Message.Append(Pad32(B));
		FBigInt HS;
		Sha1(S, HS);
		Message.Append(HS);

		Sha1(Message, OutM1);
	}

	void ComputeM2(const TArray<uint8>& A, const TArray<uint8>& M1, const TArray<uint8>& S, TArray<uint8>& OutM2)
	{
		FBigInt Message;
		Message.Append(Pad32(A));
		Message.Append(M1.GetData(), M1.Num());
		FBigInt HS;
		Sha1(S, HS);
		Message.Append(HS);
		Sha1(Message, OutM2);
	}

	void ComputeSessionKey(const TArray<uint8>& S, TArray<uint8>& OutKey)
	{
		FBigInt S32 = Pad32(S);
		uint8 Digest[20];
		Sha1(S32.GetData(), S32.Num(), Digest);

		// Interleave: байты $x$ из чётных и нечётных позиций поочерёдно.
		OutKey.Reset();
		OutKey.SetNumZeroed(40);
		for (int32 i = 0; i < 20; ++i)
		{
			OutKey[i * 2] = Digest[i];
			OutKey[i * 2 + 1] = Digest[(i + 20) % 20];
		}
	}

	void ComputeHashLE(const TArray<uint8>& Value, TArray<uint8>& OutHash)
	{
		FBigInt V = Pad32(Value);
		Sha1(V, OutHash);
	}
}
