#pragma once

class RapidFireSkill final : public Skill
{
public:
	RapidFireSkill() : Skill()
	{
		m_requiredSkillPoints = 1;

		m_hasDuration = true;
		m_durationInSeconds = 8;

		m_skillTarget = SkillTarget::SELF;

		m_isReflectableSkillCard = false;
	}

	uint32_t getItemId() override
	{
		return SKILL_RAPID_FIRE;
	}
};