#include "framework.h"
#include "Game/collision/sphere.h"

#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/Setup.h"
#include "Specific/level.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Renderer;

SPHERE LaraSpheres[MAX_SPHERES];
SPHERE CreatureSpheres[MAX_SPHERES];

int GetSpheres(ItemInfo* item, SPHERE* ptr, int worldSpace, Matrix local)
{
	if (item == nullptr)
		return 0;

	BoundingSphere spheres[MAX_SPHERES];
	short itemNumber = (item - g_Level.Items.data());

	int num = g_Renderer.GetSpheres(itemNumber, spheres, worldSpace, local);

	for (int i = 0; i < MAX_SPHERES; i++)
	{
		ptr[i].x = spheres[i].Center.x;
		ptr[i].y = spheres[i].Center.y;
		ptr[i].z = spheres[i].Center.z;
		ptr[i].r = spheres[i].Radius;
	}

	return num;
}

int TestCollision(ItemInfo* item, ItemInfo* laraItem)
{
	int flags = 0;

	int creatureSphereCount = GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
	int laraSphereCount = GetSpheres(laraItem, LaraSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);

	laraItem->TouchBits = NO_JOINT_BITS;

	if (creatureSphereCount <= 0)
	{
		item->TouchBits = NO_JOINT_BITS;
		return 0;
	}
	else
	{
		for (int i = 0; i < creatureSphereCount; i++)
		{
			auto* ptr1 = &CreatureSpheres[i];
			
			int x1 = item->Pose.Position.x + ptr1->x;
			int y1 = item->Pose.Position.y + ptr1->y;
			int z1 = item->Pose.Position.z + ptr1->z;
			int r1 = ptr1->r;

			if (r1 > 0)
			{
				for (int j = 0; j < laraSphereCount; j++)
				{
					auto* ptr2 = &LaraSpheres[j];

					int x2 = item->Pose.Position.x + ptr2->x;
					int y2 = item->Pose.Position.y + ptr2->y;
					int z2 = item->Pose.Position.z + ptr2->z;
					int r2 = ptr2->r;

					if (r2 > 0)
					{
						int dx = x1 - x2;
						int dy = y1 - y2;
						int dz = z1 - z2;
						int r = r1 + r2;

						if ((SQUARE(dx) + SQUARE(dy) + SQUARE(dz)) < SQUARE(r))
						{
							item->TouchBits.Set(i);
							laraItem->TouchBits.Set(j);
							flags |= 1 << i;
							break;
						}
					}
				}
			}
		}

		return flags;
	}
}
