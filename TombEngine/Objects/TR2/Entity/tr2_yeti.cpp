#include "framework.h"
#include "Objects/TR2/Entity/tr2_yeti.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Specific/level.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR2
{
	const auto YetiBiteLeft	 = CreatureBiteInfo(Vector3(12, 101, 19), 13);
	const auto YetiBiteRight = CreatureBiteInfo(Vector3(12, 101, 19), 10);
	const auto YetiAttackJoints1 = std::vector<unsigned int>{ 10, 12 }; // TODO: Rename.
	const auto YetiAttackJoints2 = std::vector<unsigned int>{ 8, 9, 10 };

	// TODO
	enum YetiState
	{

	};

	// TODO
	enum YetiAnim
	{

	};

	void InitializeYeti(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitializeCreature(itemNumber);
		SetAnimation(item, 19);
	}

	void YetiControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* info = GetCreatureInfo(item);

		bool isLaraAlive = LaraItem->HitPoints > 0;

		short angle = 0;
		short tilt = 0;
		short head = 0;
		short torso = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 8)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 31;
				item->Animation.FrameNumber = GetAnimData(item).frameBase;
				item->Animation.ActiveState = 8;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, info->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case 2:
				info->MaxTurn = 0;
				info->Flags = 0;

				if (AI.ahead)
					head = AI.angle;

				if (info->Mood == MoodType::Escape)
					item->Animation.TargetState = 1;
				else if (item->Animation.RequiredState != NO_STATE)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (info->Mood == MoodType::Bored)
				{
					if (Random::TestProbability(1 / 128.0f) || !isLaraAlive)
						item->Animation.TargetState = 7;
					else if (Random::TestProbability(1 / 64.0f))
						item->Animation.TargetState = 9;
					else if (Random::TestProbability(0.025f))
						item->Animation.TargetState = 3;
				}
				else if (AI.ahead && AI.distance < pow(BLOCK(0.5f), 2) && Random::TestProbability(1 / 2.0f))
					item->Animation.TargetState = 4;
				else if (AI.ahead && AI.distance < pow(CLICK(1), 2))
					item->Animation.TargetState = 5;
				else if (info->Mood == MoodType::Stalk)
					item->Animation.TargetState = 3;
				else
					item->Animation.TargetState = 1;

				break;

			case 7:
				if (AI.ahead)
					head = AI.angle;

				if (info->Mood == MoodType::Escape || item->HitStatus)
					item->Animation.TargetState = 2;
				else if (info->Mood == MoodType::Bored)
				{
					if (isLaraAlive)
					{
						if (Random::TestProbability(1 / 128.0f))
							item->Animation.TargetState = 2;
						else if (Random::TestProbability(1 / 64.0f))
							item->Animation.TargetState = 9;
						else if (Random::TestProbability(0.025f))
						{
							item->Animation.TargetState = 2;
							item->Animation.RequiredState = 3;
						}
					}
				}
				else if (Random::TestProbability(1 / 64.0f))
					item->Animation.TargetState = 2;

				break;

			case 9:
				if (AI.ahead)
					head = AI.angle;

				if (info->Mood == MoodType::Escape || item->HitStatus)
					item->Animation.TargetState = 2;
				else if (info->Mood == MoodType::Bored)
				{
					if (Random::TestProbability(1 / 128.0f) || !isLaraAlive)
						item->Animation.TargetState = 7;
					else if (Random::TestProbability(1 / 64.0f))
						item->Animation.TargetState = 2;
					else if (Random::TestProbability(0.025f))
					{
						item->Animation.TargetState = 2;
						item->Animation.RequiredState = 3;
					}
				}
				else if (Random::TestProbability(1 / 64.0f))
					item->Animation.TargetState = 2;

				break;

			case 3:
				info->MaxTurn = ANGLE(4.0f);

				if (AI.ahead)
					head = AI.angle;

				if (info->Mood == MoodType::Escape)
					item->Animation.TargetState = 1;
				else if (info->Mood == MoodType::Bored)
				{
					if (Random::TestProbability(1 / 128.0f) || !isLaraAlive)
					{
						item->Animation.TargetState = 2;
						item->Animation.RequiredState = 7;
					}
					else if (Random::TestProbability(1 / 64.0f))
					{
						item->Animation.TargetState = 2;
						item->Animation.RequiredState = 9;
					}
					else if (Random::TestProbability(0.025f))
						item->Animation.TargetState = 2;
				}
				else if (info->Mood == MoodType::Attack)
				{
					if (AI.ahead && AI.distance < pow(CLICK(1), 2))
						item->Animation.TargetState = 2;
					else if (AI.distance < pow(BLOCK(2), 2))
						item->Animation.TargetState = 1;
				}

				break;

			case 1:
				tilt = angle / 4;
				info->MaxTurn = ANGLE(6.0f);
				info->Flags = 0;

				if (AI.ahead)
					head = AI.angle;

				if (info->Mood == MoodType::Escape)
					break;
				else if (info->Mood == MoodType::Bored)
					item->Animation.TargetState = 3;
				else if (AI.ahead && AI.distance < pow(CLICK(1), 2))
					item->Animation.TargetState = 2;
				else if (AI.ahead && AI.distance < pow(BLOCK(2), 2))
					item->Animation.TargetState = 6;
				else if (info->Mood == MoodType::Stalk)
					item->Animation.TargetState = 3;

				break;

			case 4:
				if (AI.ahead)
					torso = AI.angle;

				if (!info->Flags && item->TouchBits.Test(YetiAttackJoints1))
				{
					CreatureEffect(item, YetiBiteRight, DoBloodSplat);
					DoDamage(info->Enemy, 100);
					info->Flags = 1;
				}

				break;

			case 5:
				info->MaxTurn = ANGLE(4.0f);

				if (AI.ahead)
					torso = AI.angle;

				if (!info->Flags &&
					(item->TouchBits.Test(YetiAttackJoints1) || item->TouchBits.Test(YetiAttackJoints2)))
				{
					if (item->TouchBits.Test(YetiAttackJoints2))
						CreatureEffect(item, YetiBiteLeft, DoBloodSplat);

					if (item->TouchBits.Test(YetiAttackJoints1))
						CreatureEffect(item, YetiBiteRight, DoBloodSplat);

					DoDamage(info->Enemy, 150);
					info->Flags = 1;
				}

				break;

			case 6:
				if (AI.ahead)
					torso = AI.angle;

				if (!info->Flags &&
					(item->TouchBits.Test(YetiAttackJoints1) || item->TouchBits.Test(YetiAttackJoints2)))
				{
					if (item->TouchBits.Test(YetiAttackJoints2))
						CreatureEffect(item, YetiBiteLeft, DoBloodSplat);

					if (item->TouchBits.Test(YetiAttackJoints1))
						CreatureEffect(item, YetiBiteRight, DoBloodSplat);

					DoDamage(info->Enemy, 200);
					info->Flags = 1;
				}

				break;

			case 10:
			case 11:
			case 12:
			case 13:
				info->MaxTurn = 0;
				break;
			}
		}

		if (!isLaraAlive)
		{
			info->MaxTurn = 0;
			CreatureKill(item, 31, 0, 14, 103); // TODO: add yeti state enum and lara extra state enum
			return;
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, torso);
		CreatureJoint(item, 1, head);

		if (item->Animation.ActiveState < 10)
		{
			switch (CreatureVault(itemNumber, angle, 2, 300))
			{
			case 2:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 34;
				item->Animation.FrameNumber = GetAnimData(item).frameBase;
				item->Animation.ActiveState = 10;
				break;

			case 3:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 33;
				item->Animation.FrameNumber = GetAnimData(item).frameBase;
				item->Animation.ActiveState = 11;
				break;

			case 4:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 32;
				item->Animation.FrameNumber = GetAnimData(item).frameBase;
				item->Animation.ActiveState = 12;
				break;

			case -4:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 35;
				item->Animation.FrameNumber = GetAnimData(item).frameBase;
				item->Animation.ActiveState = 13;
				break;
			}
		}
		else
			CreatureAnimation(itemNumber, angle, tilt);
	}
}
