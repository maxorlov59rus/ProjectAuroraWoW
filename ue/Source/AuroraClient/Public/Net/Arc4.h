#pragma once

#include "CoreMinimal.h"

/**
 * ARC4 (RC4) — потоковый шифр, которым шифруется трафик WorldServer.
 *
 * Используется только для протокола 3.3.5a; в криптографии считается
 * устаревшим, своих данных им защищать нельзя.
 */
namespace Aurora::World
{
	class FArc4
	{
	public:
		FArc4() { Reset(); }

		/** Инициализация ключом (KSA). */
		void SetKey(const TArray<uint8>& Key)
		{
			for (int32 i = 0; i < 256; ++i)
			{
				S[i] = static_cast<uint8>(i);
			}

			if (Key.Num() == 0)
			{
				I = 0;
				J = 0;
				return;
			}

			int32 j = 0;
			for (int32 i = 0; i < 256; ++i)
			{
				j = (j + S[i] + Key[i % Key.Num()]) & 0xFF;
				Swap(i, j);
			}
			I = 0;
			J = 0;
		}

		void Reset()
		{
			for (int32 i = 0; i < 256; ++i)
			{
				S[i] = static_cast<uint8>(i);
			}
			I = 0;
			J = 0;
		}

		/** Шифрование = расшифровывание. */
		void Process(uint8* Data, int32 Length)
		{
			for (int32 k = 0; k < Length; ++k)
			{
				I = (I + 1) & 0xFF;
				J = (J + S[I]) & 0xFF;
				Swap(I, J);
				const uint8 K = S[(S[I] + S[J]) & 0xFF];
				Data[k] ^= K;
			}
		}

		void Process(TArray<uint8>& Data)
		{
			Process(Data.GetData(), Data.Num());
		}

		/** Пропускает N байт ключевого потока (сервер делает это на старте). */
		void Drop(int32 Count)
		{
			uint8 Dummy[64];
			while (Count > 0)
			{
				const int32 Chunk = FMath::Min(Count, 64);
				FMemory::Memzero(Dummy, Chunk);
				Process(Dummy, Chunk);
				Count -= Chunk;
			}
		}

	private:
		void Swap(int32 A, int32 B)
		{
			const uint8 T = S[A];
			S[A] = S[B];
			S[B] = T;
		}

		uint8 S[256];
		int32 I = 0;
		int32 J = 0;
	};
}
