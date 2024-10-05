#ifndef MLC_CORE_OBJECT_H_
#define MLC_CORE_OBJECT_H_

#include "./utils.h"

namespace mlc {

#define MLC_DEF_REFLECTION(ObjType)                                                                                    \
  static inline const int32_t _type_reflect = ::mlc::core::ReflectionHelper(static_cast<int32_t>(ObjType::_type_index))

#define MLC_DEF_TYPE_COMMON_(SelfType, ParentType, TypeIndex, TypeKey)                                                 \
public:                                                                                                                \
  template <typename> friend struct ::mlc::base::DefaultObjectAllocator;                                               \
  template <typename> friend struct ::mlc::Ref;                                                                        \
  friend struct ::mlc::Any;                                                                                            \
  friend struct ::mlc::AnyView;                                                                                        \
  template <typename DerivedType> MLC_INLINE bool IsInstance() const {                                                 \
    return ::mlc::base::IsInstanceOf<DerivedType, SelfType>(reinterpret_cast<const MLCAny *>(this));                   \
  }                                                                                                                    \
  MLC_INLINE const char *GetTypeKey() const {                                                                          \
    return ::mlc::base::TypeIndex2TypeKey(reinterpret_cast<const MLCAny *>(this));                                     \
  }                                                                                                                    \
  [[maybe_unused]] static constexpr const char *_type_key = TypeKey;                                                   \
  [[maybe_unused]] static inline MLCTypeInfo *_type_info =                                                             \
      ::mlc::base::TypeRegister(static_cast<int32_t>(ParentType::_type_index), /**/                                    \
                                static_cast<int32_t>(TypeIndex), TypeKey,      /**/                                    \
                                &::mlc::core::ObjPtrGetter<SelfType>,          /**/                                    \
                                &::mlc::core::ObjPtrSetter<SelfType>);                                                 \
  [[maybe_unused]] static inline int32_t *_type_ancestors = _type_info->type_ancestors;                                \
  [[maybe_unused]] static constexpr int32_t _type_depth = ParentType::_type_depth + 1;                                 \
  using _type_parent [[maybe_unused]] = ParentType;                                                                    \
  static_assert(sizeof(::mlc::Ref<SelfType>) == sizeof(MLCObjPtr), "Size mismatch")

#define MLC_DEF_STATIC_TYPE(SelfType, ParentType, TypeIndex, TypeKey)                                                  \
public:                                                                                                                \
  MLC_DEF_TYPE_COMMON_(SelfType, ParentType, TypeIndex, TypeKey);                                                      \
  [[maybe_unused]] static constexpr int32_t _type_index = static_cast<int32_t>(TypeIndex);                             \
  MLC_DEF_REFLECTION(SelfType)

#define MLC_DEF_DYN_TYPE(SelfType, ParentType, TypeKey)                                                                \
public:                                                                                                                \
  MLC_DEF_TYPE_COMMON_(SelfType, ParentType, -1, TypeKey);                                                             \
  [[maybe_unused]] static inline int32_t _type_index = _type_info->type_index;                                         \
  MLC_DEF_REFLECTION(SelfType)

#define MLC_DEF_OBJ_REF(SelfType, ObjType, ParentType)                                                                 \
private:                                                                                                               \
  MLC_DEF_OBJ_PTR_METHODS_(SelfType, ObjType);                                                                         \
  MLC_INLINE void CheckNull() const {                                                                                  \
    if (this->ptr == nullptr) {                                                                                        \
      MLC_THROW(TypeError) << "Cannot convert from type `None` to non-nullable `"                                      \
                           << ::mlc::base::Type2Str<SelfType>::Run() << "`";                                           \
    }                                                                                                                  \
  }                                                                                                                    \
                                                                                                                       \
public:                                                                                                                \
  /***** Section 1. Default constructor/destructors *****/                                                             \
  MLC_INLINE ~SelfType() = default;                                                                                    \
  SelfType(std::nullptr_t) = delete;                                                                                   \
  SelfType &operator=(std::nullptr_t) = delete;                                                                        \
  MLC_INLINE SelfType(::mlc::NullType) : ParentType(::mlc::Null) {}                                                    \
  MLC_INLINE SelfType &operator=(::mlc::NullType) { return this->Reset(); }                                            \
  MLC_INLINE SelfType &Reset() {                                                                                       \
    TBase::Reset();                                                                                                    \
    return *this;                                                                                                      \
  }                                                                                                                    \
  /***** Section 2. Conversion between `SelfType` *****/                                                               \
  MLC_INLINE SelfType(const SelfType &src) : ParentType(::mlc::Null) {                                                 \
    this->_SetObjPtr(src.ptr);                                                                                         \
    this->IncRef();                                                                                                    \
  }                                                                                                                    \
  MLC_INLINE SelfType(SelfType &&src) : ParentType(::mlc::Null) {                                                      \
    this->_SetObjPtr(src.ptr);                                                                                         \
    src.ptr = nullptr;                                                                                                 \
  }                                                                                                                    \
  MLC_INLINE TSelf &operator=(const SelfType &other) {                                                                 \
    TSelf(other).Swap(*this);                                                                                          \
    return *this;                                                                                                      \
  }                                                                                                                    \
  MLC_INLINE TSelf &operator=(SelfType &&other) {                                                                      \
    TSelf(std::move(other)).Swap(*this);                                                                               \
    return *this;                                                                                                      \
  }                                                                                                                    \
  /***** Section 3. Conversion between `Ref<U>` / `U *` where `U` is derived from `ObjType` *****/                     \
  template <typename U, typename = std::enable_if_t<::mlc::base::IsDerivedFrom<U, ObjType>>>                           \
  MLC_INLINE SelfType(const ::mlc::Ref<U> &src) : ParentType(::mlc::Null) {                                            \
    this->_SetObjPtr(::mlc::base::ObjPtrHelper<::mlc::Ref<U>>::GetPtr(&src));                                          \
    this->CheckNull();                                                                                                 \
    this->IncRef();                                                                                                    \
  }                                                                                                                    \
  template <typename U, typename = std::enable_if_t<::mlc::base::IsDerivedFrom<U, ObjType>>>                           \
  MLC_INLINE SelfType(::mlc::Ref<U> &&src) : ParentType(::mlc::Null) {                                                 \
    this->_SetObjPtr(::mlc::base::ObjPtrHelper<::mlc::Ref<U>>::MovePtr(&src));                                         \
    this->CheckNull();                                                                                                 \
  }                                                                                                                    \
  template <typename U, typename = std::enable_if_t<::mlc::base::IsDerivedFrom<U, ObjType>>>                           \
  MLC_INLINE TSelf &operator=(const ::mlc::Ref<U> &other) {                                                            \
    TSelf(other).Swap(*this);                                                                                          \
    return *this;                                                                                                      \
  }                                                                                                                    \
  template <typename U, typename = std::enable_if_t<::mlc::base::IsDerivedFrom<U, ObjType>>>                           \
  MLC_INLINE TSelf &operator=(::mlc::Ref<U> &&other) {                                                                 \
    TSelf(std::move(other)).Swap(*this);                                                                               \
    return *this;                                                                                                      \
  }                                                                                                                    \
  template <typename U, typename = std::enable_if_t<::mlc::base::IsDerivedFrom<U, ObjType>>>                           \
  MLC_INLINE explicit SelfType(U *src) : ParentType(::mlc::Null) {                                                     \
    this->_SetObjPtr(reinterpret_cast<MLCObject *>(src));                                                              \
    this->CheckNull();                                                                                                 \
    this->IncRef();                                                                                                    \
  }                                                                                                                    \
  /***** Section 4. The `new` operator *****/                                                                          \
  template <typename... Args, typename = std::enable_if_t<::mlc::base::Newable<ObjType, Args...>>>                     \
  MLC_INLINE SelfType(Args &&...args)                                                                                  \
      : SelfType(::mlc::base::AllocatorOf<ObjType>::New(std::forward<Args>(args)...)) {}                               \
  /***** Section 5. Conversion between AnyView/Any *****/                                                              \
  MLC_INLINE SelfType(const ::mlc::AnyView &src) : ParentType(::mlc::Null) {                                           \
    TBase::_Init<ObjType>(src);                                                                                        \
    this->CheckNull();                                                                                                 \
  }                                                                                                                    \
  MLC_INLINE SelfType(const ::mlc::Any &src) : ParentType(::mlc::Null) {                                               \
    TBase::_Init<ObjType>(src);                                                                                        \
    this->CheckNull();                                                                                                 \
  }                                                                                                                    \
  MLC_INLINE operator ::mlc::AnyView() const {                                                                         \
    if (this->ptr != nullptr) {                                                                                        \
      ::mlc::AnyView ret;                                                                                              \
      ret.type_index = this->ptr->type_index;                                                                          \
      ret.v_obj = this->ptr;                                                                                           \
      return ret;                                                                                                      \
    }                                                                                                                  \
    return ::mlc::AnyView();                                                                                           \
  };                                                                                                                   \
  MLC_INLINE operator ::mlc::Any() const & {                                                                           \
    if (this->ptr != nullptr) {                                                                                        \
      ::mlc::Any ret;                                                                                                  \
      ret.type_index = this->ptr->type_index;                                                                          \
      ret.v_obj = this->ptr;                                                                                           \
      ::mlc::base::IncRef(this->ptr);                                                                                  \
      return ret;                                                                                                      \
    }                                                                                                                  \
    return ::mlc::Any();                                                                                               \
  }                                                                                                                    \
  MLC_INLINE operator ::mlc::Any() && {                                                                                \
    if (this->ptr != nullptr) {                                                                                        \
      ::mlc::Any ret;                                                                                                  \
      ret.type_index = this->ptr->type_index;                                                                          \
      ret.v_obj = this->ptr;                                                                                           \
      this->ptr = nullptr;                                                                                             \
      return ret;                                                                                                      \
    }                                                                                                                  \
    return ::mlc::Any();                                                                                               \
  }                                                                                                                    \
  MLC_DEF_REFLECTION(ObjType)

struct Object {
  MLCAny _mlc_header;

  MLC_INLINE Object() : _mlc_header() {}
  MLC_INLINE Object(const Object &) : _mlc_header() {}
  MLC_INLINE Object(Object &&) {}
  MLC_INLINE Object &operator=(const Object &) { return *this; }
  MLC_INLINE Object &operator=(Object &&) { return *this; }
  Str str() const;
  friend std::ostream &operator<<(std::ostream &os, const Object &src);

  MLC_DEF_STATIC_TYPE(Object, ::mlc::base::ObjectDummyRoot, MLCTypeIndex::kMLCObject, "object.Object");
};

struct ObjectRef : protected ::mlc::base::ObjectRefDummyRoot {
  MLC_DEF_OBJ_REF(ObjectRef, Object, ::mlc::base::ObjectRefDummyRoot) //
      .StaticFn("__init__", InitOf<Object>);
};

} // namespace mlc

#endif // MLC_CORE_OBJECT_H_
