#include "AudioInstanceObjectBinding.h"

// Move assignment for object audio bindings with a noexcept guarantee to ensure it is used by STL containers
AudioInstanceObjectBinding & AudioInstanceObjectBinding::operator=(const AudioInstanceObjectBinding && other) noexcept
{
	m_object = std::move(other.m_object);
	m_obj_id = other.m_obj_id;
	m_item_id = other.m_item_id;
	m_identifier = other.m_identifier;
	m_max_dist_sq = other.m_max_dist_sq;
	m_last_valid = other.m_last_valid;
	return *this;
}