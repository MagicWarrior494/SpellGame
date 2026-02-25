#pragma once
#include <vector>

class PushConstantsData
{
public:
	std::vector<uint8_t> buffer;

	void Reset(size_t size)
	{
		buffer.resize(size);
	}

	template<typename T>
	void Set(size_t offset, const T& data)
	{
		if (offset + sizeof(T) > buffer.size())
			throw std::out_of_range("PushConstantsData: Offset out of range");
		std::memcpy(buffer.data() + offset, &data, sizeof(T));
	}
};