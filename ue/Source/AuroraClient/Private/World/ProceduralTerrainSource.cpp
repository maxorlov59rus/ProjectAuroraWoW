#include "World/ProceduralTerrainSource.h"

namespace Aurora::World
{
	float FProceduralTerrainSource::SampleHeight(float X, float Y) const
	{
		// Две косинусоиды с несоизмеримыми периодами дают достаточное
		// разнообразие рельефа без таблиц шума и без сторонних зависимостей.
		const float A = FMath::Cos(X * Params.Frequency) * FMath::Cos(Y * Params.Frequency * 0.7f);
		const float B = FMath::Sin(X * Params.Frequency * 2.3f + 1.7f)
			* FMath::Sin(Y * Params.Frequency * 1.9f + 0.4f) * 0.4f;

		return (A + B) * Params.Amplitude;
	}

	bool FProceduralTerrainSource::GetTerrain(float CenterX, float CenterY, float /*Radius*/,
	                                          TArray<FTerrainChunk>& OutChunks)
	{
		OutChunks.Reset();

		const int32 Cells = FMath::Max(Params.Cells, 1);
		const float CellSize = FMath::Max(Params.CellSize, 1.f);

		FTerrainChunk Chunk;
		Chunk.Rows = Cells + 1;
		Chunk.Cols = Cells + 1;
		Chunk.Vertices.Reserve(Chunk.Rows * Chunk.Cols);

		const float OriginX = CenterX - (Cells * CellSize) * 0.5f;
		const float OriginY = CenterY - (Cells * CellSize) * 0.5f;

		for (int32 Row = 0; Row < Chunk.Rows; ++Row)
		{
			for (int32 Col = 0; Col < Chunk.Cols; ++Col)
			{
				FTerrainVertex Vertex;
				Vertex.X = OriginX + Col * CellSize;
				Vertex.Y = OriginY + Row * CellSize;
				Vertex.Z = SampleHeight(Vertex.X, Vertex.Y);
				Chunk.Vertices.Add(Vertex);
			}
		}

		OutChunks.Add(MoveTemp(Chunk));
		return true;
	}

	bool FProceduralTerrainSource::GetEntities(float /*CenterX*/, float /*CenterY*/, float /*Radius*/,
	                                           TArray<FWorldEntity>& OutEntities)
	{
		// Моделей нет: процедурный источник отвечает только за рельеф.
		OutEntities.Reset();
		return true;
	}
}
