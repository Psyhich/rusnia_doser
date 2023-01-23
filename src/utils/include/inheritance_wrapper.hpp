#ifndef INHERITANCE_WRAPPER_H
#define INHERITANCE_WRAPPER_H

#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

template<typename BaseT, std::size_t CONTAINER_SIZE=sizeof(BaseT)>
class InheritanceWrapper
{
public:
	template<typename InheritedT,
		std::enable_if_t<std::is_base_of_v<BaseT, InheritedT>, bool> = true>
	InheritanceWrapper(InheritedT &&valueToPlaceInside)
	{
		if constexpr (std::is_move_constructible_v<InheritedT>)
		{
			emplaceWithoutDeletion<InheritedT>(std::move(valueToPlaceInside));
		}
		else if constexpr (std::is_copy_constructible_v<InheritedT>)
		{
			emplaceWithoutDeletion<InheritedT>(valueToPlaceInside);
		}
		else
		{
			static_assert("Given type must be move or copy constructible");
		}
	}

	~InheritanceWrapper()
	{
		destruct();
	}

	constexpr const BaseT *operator->() const
	{
		return GetInternalType();
	}
	constexpr BaseT *operator->()
	{
		return GetInternalType();
	}
	constexpr const BaseT *operator *() const
	{
		return GetInternalType();
	}
	constexpr BaseT *operator *()
	{
		return GetInternalType();
	}

	template<typename InheritedT, typename ...Args,
		std::enable_if_t<std::is_base_of_v<BaseT, InheritedT>, bool> = true>
	void emplace(Args &&...constructionArgs)
	{
		destruct();
		emplaceWithoutDeletion<InheritedT>(std::forward<Args>(constructionArgs)...);
	}

private:
	inline const BaseT *GetInternalType() const
	{
		return reinterpret_cast<const BaseT *>(m_holdingBytes.data());
	}
	inline BaseT *GetInternalType()
	{
		return reinterpret_cast<BaseT *>(m_holdingBytes.data());
	}

	void destruct()
	{
		GetInternalType()->~BaseT();
	}

	template<typename InheritedT, typename ...Args,
		std::enable_if_t<std::is_base_of_v<BaseT, InheritedT>, bool> = true>
	void emplaceWithoutDeletion(Args &&...constructionArgs)
	{
		static_assert(std::is_base_of_v<BaseT, InheritedT>, "Given type doesn't derive from this container type");
		static_assert(sizeof(InheritedT) <= CONTAINER_SIZE, "There is not enough size fot this type");

		// It's C++17, so no concepts today(
		::new(m_holdingBytes.data()) InheritedT(std::forward<Args>(constructionArgs)...);
	}

	alignas(BaseT) std::array<std::uint8_t, CONTAINER_SIZE> m_holdingBytes;
};

#endif // INHERITANCE_WRAPPER_H
