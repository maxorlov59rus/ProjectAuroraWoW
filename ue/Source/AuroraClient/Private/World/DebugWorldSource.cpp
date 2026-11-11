#include "World/DebugWorldSource.h"

namespace Aurora::World
{
	bool FDebugWorldSource::GetTerrain(float CenterX, float CenterY, float /*Radius*/,
	                                   TArray<FTerrainChunk>& OutChunks)
	{
		OutChunks.Reset();

		FTerrainChunk Chunk;
		Chunk.Rows = GridCells + 1;
		Chunk.Cols = GridCells + 1;
		Chunk.Vertices.Reserve(Chunk.Rows * Chunk.Cols);

		// Плоская сетка с шагом GridStep, центрированная на точке входа.
		// Z = 0: высот карты пока нет, это заведомо отладочная плоскость.
		const float OriginX = CenterX - (GridCells * GridStep) * 0.5f;
		const float OriginY = CenterY - (GridCells * GridStep) * 0.5f;

		for (int32 Row = 0; Row < Chunk.Rows; ++Row)
		{
			for (int32 Col = 0; Col < Chunk.Cols; ++Col)
			{
				FTerrainVertex Vertex;
				Vertex.X = OriginX + Col * GridStep;
				Vertex.Y = OriginY + Row * GridStep;
				Vertex.Z = 0.f;
				Chunk.Vertices.Add(Vertex);
			}
		}

		OutChunks.Add(MoveTemp(Chunk));
		return true;
	}

	bool FDebugWorldSource::GetEntities(float CenterX, float CenterY, float Radius,
	                                    TArray<FWorldEntity>& OutEntities)
	{
		OutEntities.Reset();

		const float RadiusSq = Radius * Radius;

		for (const FWorldEntity& Entity : Entities)
		{
			const float Dx = Entity.X - CenterX;
			const float Dy = Entity.Y - CenterY;

			if (Dx * Dx + Dy * Dy <= RadiusSq)
			{
				OutEntities.Add(Entity);
			}
		}

		return true;
	}
}
