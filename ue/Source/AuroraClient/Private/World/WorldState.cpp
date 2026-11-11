#include "World/WorldState.h"

namespace Aurora::World
{
	int32 FWorldState::ApplyUpdateObject(const TArray<uint8>& Body)
	{
		TArray<FUpdateEvent> Events;
		TArray<uint64> Destroyed;

		if (!ParseUpdateObject(Body, Events, Destroyed))
		{
			return 0;
		}

		int32 Changed = 0;

		for (const uint64 Guid : Destroyed)
		{
			if (Objects.Remove(Guid) > 0)
			{
				++Changed;
			}
		}

		for (const FUpdateEvent& Event : Events)
		{
			if (Event.Guid == 0)
			{
				continue;
			}

			const bool bCreates =
				Event.Block == EUpdateBlock::CreateObject ||
				Event.Block == EUpdateBlock::CreateObject2;

			FWorldObject* Existing = Objects.Find(Event.Guid);

			if (!Existing)
			{
				if (!bCreates && Event.Block != EUpdateBlock::Values &&
					Event.Block != EUpdateBlock::Movement)
				{
					continue;
				}

				FWorldObject Created;
				Created.Guid = Event.Guid;
				Created.Kind = KindFromTypeMask(Event.TypeMask);
				Created.X = Event.X;
				Created.Y = Event.Y;
				Created.Z = Event.Z;
				Created.Orientation = Event.Orientation;
				Objects.Add(Event.Guid, MoveTemp(Created));
				++Changed;
				continue;
			}

			// Movement и значения обновляют координаты, если они пришли.
			if (Event.Block == EUpdateBlock::Movement || bCreates)
			{
				Existing->X = Event.X;
				Existing->Y = Event.Y;
				Existing->Z = Event.Z;
				Existing->Orientation = Event.Orientation;
				++Changed;
			}

			if (bCreates && Event.TypeMask != 0)
			{
				Existing->Kind = KindFromTypeMask(Event.TypeMask);
			}

			if (!Event.Name.IsEmpty())
			{
				Existing->Name = Event.Name;
			}
		}

		return Changed;
	}

	bool FWorldState::Remove(uint64 Guid)
	{
		return Objects.Remove(Guid) > 0;
	}

	void FWorldState::Clear()
	{
		Objects.Reset();
	}

	const FWorldObject* FWorldState::Find(uint64 Guid) const
	{
		return Objects.Find(Guid);
	}

	void FWorldState::GetEntitiesNear(float CenterX, float CenterY, float Radius,
	                                  TArray<FWorldEntity>& OutEntities) const
	{
		OutEntities.Reset();

		const float RadiusSq = Radius * Radius;

		for (const TPair<uint64, FWorldObject>& Pair : Objects)
		{
			const FWorldObject& Object = Pair.Value;

			const float Dx = Object.X - CenterX;
			const float Dy = Object.Y - CenterY;

			if (Dx * Dx + Dy * Dy > RadiusSq)
			{
				continue;
			}

			FWorldEntity Entity;
			Entity.Guid = Object.Guid;
			Entity.Kind = Object.Kind;
			Entity.Name = Object.Name;
			Entity.X = Object.X;
			Entity.Y = Object.Y;
			Entity.Z = Object.Z;
			Entity.Orientation = Object.Orientation;

			OutEntities.Add(MoveTemp(Entity));
		}
	}
}
