#pragma once

#include "CoreMinimal.h"

/**
 * SRP6 для протокола AzerothCore (3.3.5a).
 *
 * BigInt представлен массивом uint8 в little-endian — в таком виде его
 * ожидает и AuthServer, и база account.salt/verifier.
 *
 * Все операции написаны вручную, без сторонних библиотек: UE не несёт
 * big-integer арифметики, а тянуть её ради двух вычислений при логине
 * нет смысла.
 */
namespace Aurora::Srp6
{
	/** v = g^x mod N. */
	void ComputeVerifier(
		const TArray<uint8>& Salt,
		const TArray<uint8>& UserPasswordHash,
		TArray<uint8>& OutVerifier);

	/** x = SHA1(salt | SHA1(USER:PASS)), little-endian. */
	void ComputeX(
		const TArray<uint8>& Salt,
		const TArray<uint8>& UserPasswordHash,
		TArray<uint8>& OutX);

	/** A = g^a mod N, где a — случайные 19 байт. */
	void GeneratePublicA(TArray<uint8>& OutA, TArray<uint8>& OutPrivateA);

	/** u = SHA1(A | B). */
	void ComputeU(const TArray<uint8>& A, const TArray<uint8>& B, TArray<uint8>& OutU);

	/** S = (B - k*g^x)^(a + u*x) mod N. */
	void ComputeS(
		const TArray<uint8>& B,
		const TArray<uint8>& PrivateA,
		const TArray<uint8>& U,
		const TArray<uint8>& X,
		TArray<uint8>& OutS);

	/** M1 = SHA1(H(N) xor H(g) | H(USER) | s | A | B | H(S)). */
	void ComputeM1(
		const TArray<uint8>& Username,
		const TArray<uint8>& Salt,
		const TArray<uint8>& A,
		const TArray<uint8>& B,
		const TArray<uint8>& S,
		TArray<uint8>& OutM1);

	/** M2 = SHA1(A | M1 | H(S)). */
	void ComputeM2(
		const TArray<uint8>& A,
		const TArray<uint8>& M1,
		const TArray<uint8>& S,
		TArray<uint8>& OutM2);

	/** Interleave-хеш S -> 40-байтовый session key. */
	void ComputeSessionKey(const TArray<uint8>& S, TArray<uint8>& OutKey);

	/** H(USER:PASS) с байтами разделителя ':' и '\0'. */
	void ComputeUserPasswordHash(
		const FString& UsernameUpper,
		const FString& Password,
		TArray<uint8>& OutHash);

	/** H(v) в том виде, в каком его сравнивает сервер: LE-байты значения. */
	void ComputeHashLE(const TArray<uint8>& Value, TArray<uint8>& OutHash);
}
