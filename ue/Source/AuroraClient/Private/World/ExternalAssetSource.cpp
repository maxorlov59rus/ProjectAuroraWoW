#include "World/ExternalAssetSource.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace Aurora::World
{
	namespace
	{
		bool ReadJsonFile(const FString& FullPath, TSharedPtr<FJsonObject>& OutJson)
		{
			FString Raw;
			if (!FFileHelper::LoadFileToString(Raw, *FullPath))
			{
				return false;
			}

			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
			return FJsonSerializer::Deserialize(Reader, OutJson) && OutJson.IsValid();
		}
	}

	bool FExternalAssetSource::LoadManifest(FString& OutError)
	{
		Chunks.Reset();
		EntityFiles.Reset();
		OutError.Reset();

		const FString ManifestPath = FPaths::Combine(RootDir, TEXT("manifest.json"));
		if (!FPaths::FileExists(ManifestPath))
		{
			OutError = FString::Printf(TEXT("manifest.json not found in %s"), *RootDir);
			return false;
		}

		TSharedPtr<FJsonObject> Manifest;
		if (!ReadJsonFile(ManifestPath, Manifest))
		{
			OutError = TEXT("manifest.json is not valid JSON");
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* Files = nullptr;
		if (!Manifest->TryGetArrayField(TEXT("files"), Files) || Files == nullptr)
		{
			OutError = TEXT("manifest.json has no 'files' array");
			return false;
		}

		int32 Accepted = 0;
		int32 Rejected = 0;

		for (const TSharedPtr<FJsonValue>& Value : *Files)
		{
			const TSharedPtr<FJsonObject> Entry = Value.IsValid() ? Value->AsObject() : nullptr;
			if (!Entry.IsValid())
			{
				continue;
			}

			FString Path;
			FString License;
			FString Map;

			if (!Entry->TryGetStringField(TEXT("path"), Path))
			{
				continue;
			}
			Entry->TryGetStringField(TEXT("license"), License);
			Entry->TryGetStringField(TEXT("map"), Map);

			// Файл без явной допустимой лицензии не загружается никогда:
			// так источник не превращается в способ распространять чужой
			// защищённый контент.
			if (!ExternalAssetPolicy::IsAllowedLicense(License))
			{
				++Rejected;
				continue;
			}

			++Accepted;

			if (Path.StartsWith(TEXT("entities/")))
			{
				if (!Map.IsEmpty())
				{
					EntityFiles.Add(Map, Path);
				}
			}
			else if (!Map.IsEmpty())
			{
				Chunks.FindOrAdd(Map).Add(Path);
			}
		}

		UE_LOG(LogTemp, Display,
			TEXT("Aurora external assets: %d accepted, %d rejected by license policy"),
			Accepted, Rejected);

		if (Accepted == 0)
		{
			OutError = TEXT("No asset passed the license check");
			return false;
		}

		return true;
	}

	bool FExternalAssetSource::TryReadLicensedFile(const FString& RelativePath,
	                                              TSharedPtr<FJsonObject>& OutJson) const
	{
		const FString FullPath = FPaths::Combine(RootDir, RelativePath);
		return FPaths::FileExists(FullPath) && ReadJsonFile(FullPath, OutJson);
	}

	bool FExternalAssetSource::GetTerrain(float CenterX, float CenterY, float Radius,
	                                      TArray<FTerrainChunk>& OutChunks)
	{
		OutChunks.Reset();

		const float RadiusSq = Radius * Radius;

		for (const TPair<FString, TArray<FString>>& MapEntry : Chunks)
		{
			for (const FString& RelativePath : MapEntry.Value)
			{
				TSharedPtr<FJsonObject> Json;
				if (!TryReadLicensedFile(RelativePath, Json))
				{
					continue;
				}

				const TArray<TSharedPtr<FJsonValue>>* Vertices = nullptr;
				if (!Json->TryGetArrayField(TEXT("vertices"), Vertices) || Vertices == nullptr)
				{
					continue;
				}

				FTerrainChunk Chunk;
				Chunk.Rows = Json->GetIntegerField(TEXT("rows"));
				Chunk.Cols = Json->GetIntegerField(TEXT("cols"));

				if (Chunk.Rows <= 0 || Chunk.Cols <= 0)
				{
					continue;
				}

				Chunk.Vertices.Reserve(Chunk.Rows * Chunk.Cols);

				float SumX = 0.f;
				float SumY = 0.f;
				int32 Count = 0;

				for (const TSharedPtr<FJsonValue>& VertexValue : *Vertices)
				{
					const TSharedPtr<FJsonObject> VertexObj =
						VertexValue.IsValid() ? VertexValue->AsObject() : nullptr;
					if (!VertexObj.IsValid())
					{
						continue;
					}

					FTerrainVertex Vertex;
					Vertex.X = static_cast<float>(VertexObj->GetNumberField(TEXT("x")));
					Vertex.Y = static_cast<float>(VertexObj->GetNumberField(TEXT("y")));
					Vertex.Z = static_cast<float>(VertexObj->GetNumberField(TEXT("z")));

					SumX += Vertex.X;
					SumY += Vertex.Y;
					++Count;

					Chunk.Vertices.Add(Vertex);
				}

				if (Count == 0)
				{
					continue;
				}

				// Чанк отбрасывается, если целиком лежит далеко от точки входа.
				const float Dx = (SumX / Count) - CenterX;
				const float Dy = (SumY / Count) - CenterY;
				if (Dx * Dx + Dy * Dy > RadiusSq)
				{
					continue;
				}

				OutChunks.Add(MoveTemp(Chunk));
			}
		}

		return OutChunks.Num() > 0;
	}

	bool FExternalAssetSource::GetEntities(float CenterX, float CenterY, float Radius,
	                                       TArray<FWorldEntity>& OutEntities)
	{
		OutEntities.Reset();

		const float RadiusSq = Radius * Radius;

		for (const TPair<FString, FString>& Entry : EntityFiles)
		{
			TSharedPtr<FJsonObject> Json;
			if (!TryReadLicensedFile(Entry.Value, Json))
			{
				continue;
			}

			const TArray<TSharedPtr<FJsonValue>>* Entities = nullptr;
			if (!Json->TryGetArrayField(TEXT("entities"), Entities) || Entities == nullptr)
			{
				continue;
			}

			for (const TSharedPtr<FJsonValue>& EntityValue : *Entities)
			{
				const TSharedPtr<FJsonObject> EntityObj =
					EntityValue.IsValid() ? EntityValue->AsObject() : nullptr;
				if (!EntityObj.IsValid())
				{
					continue;
				}

				FWorldEntity Entity;
				Entity.Guid = static_cast<uint64>(EntityObj->GetNumberField(TEXT("guid")));
				Entity.X = static_cast<float>(EntityObj->GetNumberField(TEXT("x")));
				Entity.Y = static_cast<float>(EntityObj->GetNumberField(TEXT("y")));
				Entity.Z = static_cast<float>(EntityObj->GetNumberField(TEXT("z")));
				Entity.Orientation = static_cast<float>(EntityObj->GetNumberField(TEXT("o")));

				EntityObj->TryGetStringField(TEXT("name"), Entity.Name);

				FString Kind;
				if (EntityObj->TryGetStringField(TEXT("kind"), Kind))
				{
					if (Kind.Equals(TEXT("player"), ESearchCase::IgnoreCase))
					{
						Entity.Kind = EEntityKind::Player;
					}
					else if (Kind.Equals(TEXT("npc"), ESearchCase::IgnoreCase))
					{
						Entity.Kind = EEntityKind::Npc;
					}
					else if (Kind.Equals(TEXT("gameobject"), ESearchCase::IgnoreCase))
					{
						Entity.Kind = EEntityKind::GameObject;
					}
				}

				const float Dx = Entity.X - CenterX;
				const float Dy = Entity.Y - CenterY;

				if (Dx * Dx + Dy * Dy <= RadiusSq)
				{
					OutEntities.Add(MoveTemp(Entity));
				}
			}
		}

		return OutEntities.Num() > 0;
	}
}
