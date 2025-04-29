[[nodiscard]] constexpr auto const& indexmap() const noexcept { return lay; }

[[nodiscard]] storage_t const& storage() const& noexcept { return sto; }

[[nodiscard]] storage_t& storage() & noexcept { return sto; }

[[nodiscard]] storage_t storage() && noexcept { return std::move(sto); }

[[nodiscard]] constexpr auto stride_order() const noexcept { return lay.stride_order; }

[[nodiscard]] ValueType const* data() const noexcept { return sto.data(); }

[[nodiscard]] ValueType* data() noexcept { return sto.data(); }

[[nodiscard]] auto const& shape() const noexcept { return lay.lengths(); }

[[nodiscard]] auto const& strides() const noexcept { return lay.strides(); }

[[nodiscard]] long size() const noexcept { return lay.size(); }

[[nodiscard]] long is_contiguous() const noexcept { return lay.is_contiguous(); }

[[nodiscard]] bool empty() const { return sto.is_null(); }

[[nodiscard]] bool is_empty() const noexcept { return sto.is_null(); }

[[nodiscard]] long extent(int i) const noexcept
{
#ifdef ENDA_ENFORCE_BOUNDCHECK
    if (i < 0 || i >= rank)
    {
        std::cerr << "Error in extent: Dimension " << i << " is incompatible with array of rank " << rank << std::endl;
        std::terminate();
    }
#endif
    return lay.lengths()[i];
}

[[nodiscard]] long shape(int i) const noexcept { return extent(i); }

[[nodiscard]] auto indices() const noexcept { return itertools::product_range(shape()); }

static constexpr bool is_stride_order_C() noexcept { return layout_t::is_stride_order_C(); }

static constexpr bool is_stride_order_Fortran() noexcept { return layout_t::is_stride_order_Fortran(); }

decltype(auto) operator()(_linear_index_t idx) const noexcept
{
    if constexpr (layout_t::layout_prop == layout_prop_e::strided_1d)
        return sto[idx.value * lay.min_stride()];
    else if constexpr (layout_t::layout_prop == layout_prop_e::contiguous)
        return sto[idx.value];
    else
        static_assert(always_false<layout_t>, "Internal error in array/view: Calling this type with a _linear_index_t is not allowed");
}

decltype(auto) operator()(_linear_index_t idx) noexcept
{
    if constexpr (layout_t::layout_prop == layout_prop_e::strided_1d)
        return sto[idx.value * lay.min_stride()];
    else if constexpr (layout_t::layout_prop == layout_prop_e::contiguous)
        return sto[idx.value];
    else
        static_assert(always_false<layout_t>, "Internal error in array/view: Calling this type with a _linear_index_t is not allowed");
}

private:
#ifdef ENDA_ENFORCE_BOUNDCHECK
static constexpr bool has_no_boundcheck = false;
#else
static constexpr bool has_no_boundcheck = true;
#endif

public:
template<char ResultAlgebra, bool SelfIsRvalue, typename Self, typename... Ts>
FORCEINLINE static decltype(auto) call(Self&& self, Ts const&... idxs) noexcept(has_no_boundcheck)
{
    // resulting value type
    using r_v_t = std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, ValueType const, ValueType>;

    if constexpr (sizeof...(Ts) == 0)
    {
        // if no arguments are given, a full view is returned
        return basic_array_view<r_v_t, Rank, LayoutPolicy, Algebra, AccessorPolicy, OwningPolicy> {self.lay, self.sto};
    }
    else
    {
        // otherwise we check the arguments and either access a single element or make a slice
        static_assert(((layout_t::template argument_is_allowed_for_call_or_slice<Ts> + ...) > 0),
                      "Error in array/view: Slice arguments must be convertible to range, ellipsis, or long (or string if the layout permits it)");

        // number of arguments convertible to long
        static constexpr int n_args_long = (layout_t::template argument_is_allowed_for_call<Ts> + ...);

        if constexpr (n_args_long == rank)
        {
            // access a single element
            long offset = self.lay(idxs...);
            if constexpr (is_view or not SelfIsRvalue)
            {
                // if the calling object is a view or an lvalue, we return a reference
                return AccessorPolicy::template accessor<ValueType>::access(self.sto.data(), offset);
            }
            else
            {
                // otherwise, we return a copy of the value
                return ValueType {self.sto[offset]};
            }
        }
        else
        {
            // access a slice of the view/array
            auto const [offset, idxm]      = self.lay.slice(idxs...);
            static constexpr auto res_rank = decltype(idxm)::rank();
            // resulting algebra
            static constexpr char newAlgebra = (ResultAlgebra == 'M' and (res_rank == 1) ? 'V' : ResultAlgebra);
            // resulting layout policy
            using r_layout_p = typename detail::layout_to_policy<std::decay_t<decltype(idxm)>>::type;
            return basic_array_view<ValueType, res_rank, r_layout_p, newAlgebra, AccessorPolicy, OwningPolicy> {std::move(idxm), {self.sto, offset}};
        }
    }
}

public:
template<typename... Ts>
FORCEINLINE decltype(auto) operator()(Ts const&... idxs) const& noexcept(has_no_boundcheck)
{
    static_assert((rank == -1) or (sizeof...(Ts) == rank) or (sizeof...(Ts) == 0) or (ellipsis_is_present<Ts...> and (sizeof...(Ts) <= rank + 1)),
                  "Error in array/view: Incorrect number of parameters in call operator");
    return call<Algebra, false>(*this, idxs...);
}

template<typename... Ts>
FORCEINLINE decltype(auto) operator()(Ts const&... idxs) & noexcept(has_no_boundcheck)
{
    static_assert((rank == -1) or (sizeof...(Ts) == rank) or (sizeof...(Ts) == 0) or (ellipsis_is_present<Ts...> and (sizeof...(Ts) <= rank + 1)),
                  "Error in array/view: Incorrect number of parameters in call operator");
    return call<Algebra, false>(*this, idxs...);
}

template<typename... Ts>
FORCEINLINE decltype(auto) operator()(Ts const&... idxs) && noexcept(has_no_boundcheck)
{
    static_assert((rank == -1) or (sizeof...(Ts) == rank) or (sizeof...(Ts) == 0) or (ellipsis_is_present<Ts...> and (sizeof...(Ts) <= rank + 1)),
                  "Error in array/view: Incorrect number of parameters in call operator");
    return call<Algebra, true>(*this, idxs...);
}

template<typename T>
decltype(auto) operator[](T const& idx) const& noexcept(has_no_boundcheck)
{
    static_assert((rank == 1), "Error in array/view: Subscript operator is only available for rank 1 views/arrays in C++17/20");
    return call<Algebra, false>(*this, idx);
}

template<typename T>
decltype(auto) operator[](T const& x) & noexcept(has_no_boundcheck)
{
    static_assert((rank == 1), "Error in array/view: Subscript operator is only available for rank 1 views/arrays in C++17/20");
    return call<Algebra, false>(*this, x);
}

template<typename T>
decltype(auto) operator[](T const& x) && noexcept(has_no_boundcheck)
{
    static_assert((rank == 1), "Error in array/view: Subscript operator is only available for rank 1 views/arrays in C++17/20");
    return call<Algebra, true>(*this, x);
}

static constexpr int iterator_rank = (has_strided_1d(layout_t::layout_prop) ? 1 : Rank);

using const_iterator = array_iterator<iterator_rank, ValueType const, typename AccessorPolicy::template accessor<ValueType>::pointer>;

using iterator = array_iterator<iterator_rank, ValueType, typename AccessorPolicy::template accessor<ValueType>::pointer>;

private:
template<typename Iterator>
[[nodiscard]] auto make_iterator(bool at_end) const noexcept
{
    if constexpr (iterator_rank == Rank)
    {
        // multi-dimensional iterator
        if constexpr (layout_t::is_stride_order_C())
        {
            // C-order case (array_iterator already traverses the data in C-order)
            return Iterator {indexmap().lengths(), indexmap().strides(), sto.data(), at_end};
        }
        else
        {
            // general case (we need to permute the shape and the strides according to the stride order of the layout)
            return Iterator {enda::permutations::apply(layout_t::stride_order, indexmap().lengths()),
                             enda::permutations::apply(layout_t::stride_order, indexmap().strides()),
                             sto.data(),
                             at_end};
        }
    }
    else
    {
        // 1-dimensional iterator
        return Iterator {std::array<long, 1> {size()}, std::array<long, 1> {indexmap().min_stride()}, sto.data(), at_end};
    }
}

public:
[[nodiscard]] const_iterator begin() const noexcept { return make_iterator<const_iterator>(false); }

[[nodiscard]] const_iterator cbegin() const noexcept { return make_iterator<const_iterator>(false); }

iterator begin() noexcept { return make_iterator<iterator>(false); }

[[nodiscard]] const_iterator end() const noexcept { return make_iterator<const_iterator>(true); }

[[nodiscard]] const_iterator cend() const noexcept { return make_iterator<const_iterator>(true); }

iterator end() noexcept { return make_iterator<iterator>(true); }

template<typename RHS>
auto& operator+=(RHS const& rhs) noexcept
{
    static_assert(not is_const, "Error in array/view: Can not assign to a const view");
    return operator=(*this + rhs);
}

template<typename RHS>
auto& operator-=(RHS const& rhs) noexcept
{
    static_assert(not is_const, "Error in array/view: Can not assign to a const view");
    return operator=(*this - rhs);
}

template<typename RHS>
auto& operator*=(RHS const& rhs) noexcept
{
    static_assert(not is_const, "Error in array/view: Can not assign to a const view");
    return operator=((*this) * rhs);
}

template<typename RHS>
auto& operator/=(RHS const& rhs) noexcept
{
    static_assert(not is_const, "Error in array/view: Can not assign to a const view");
    return operator=(*this / rhs);
}

template<std::ranges::contiguous_range R>
auto& operator=(R const& rhs) noexcept requires(Rank == 1 and not MemoryArray<R>)
{
    *this = basic_array_view {rhs};
    return *this;
}

private:
template<typename RHS>
void assign_from_ndarray(RHS const& rhs)
{
#ifdef ENDA_ENFORCE_BOUNDCHECK
    if (this->shape() != rhs.shape())
        ENDA_RUNTIME_ERROR << "Error in assign_from_ndarray: Size mismatch:"
                           << "\n LHS.shape() = " << this->shape() << "\n RHS.shape() = " << rhs.shape();
#endif
    // compile-time check if assignment is possible
    static_assert(std::is_assignable_v<value_type&, get_value_t<RHS>>, "Error in assign_from_ndarray: Incompatible value types");

    // are both operands enda::MemoryArray types?
    static constexpr bool both_in_memory = MemoryArray<self_t> and MemoryArray<RHS>;

    // do both operands have the same stride order?
    static constexpr bool same_stride_order = get_layout_info<self_t>.stride_order == get_layout_info<RHS>.stride_order;

    // prefer optimized options if possible
    if constexpr (both_in_memory and same_stride_order)
    {
        if (rhs.empty())
            return;
        // are both operands strided in 1d?
        static constexpr bool both_1d_strided = has_layout_strided_1d<self_t> and has_layout_strided_1d<RHS>;
        if constexpr (mem::on_host<self_t, RHS> and both_1d_strided)
        {
            // vectorizable copy on host
            for (long i = 0; i < size(); ++i)
                (*this)(_linear_index_t {i}) = rhs(_linear_index_t {i});
            return;
        }
        else if constexpr (!mem::on_host<self_t, RHS> and have_same_value_type_v<self_t, RHS>)
        {
            // check for block-layout and use mem::memcpy2D if possible
            auto bl_layout_dst = get_block_layout(*this);
            auto bl_layout_src = get_block_layout(rhs);
            if (bl_layout_dst && bl_layout_src)
            {
                auto [n_bl_dst, bl_size_dst, bl_str_dst] = *bl_layout_dst;
                auto [n_bl_src, bl_size_src, bl_str_src] = *bl_layout_src;
                // check that the total memory size is the same
                if (n_bl_dst * bl_size_dst != n_bl_src * bl_size_src)
                    ENDA_RUNTIME_ERROR << "Error in assign_from_ndarray: Incompatible block sizes";
                // if either destination or source consists of a single block, we can chunk it up to make the layouts compatible
                if (n_bl_dst == 1 && n_bl_src > 1)
                {
                    n_bl_dst = n_bl_src;
                    bl_size_dst /= n_bl_src;
                    bl_str_dst = bl_size_dst;
                }
                if (n_bl_src == 1 && n_bl_dst > 1)
                {
                    n_bl_src = n_bl_dst;
                    bl_size_src /= n_bl_dst;
                    bl_str_src = bl_size_src;
                }
                // copy only if block-layouts are compatible, otherwise continue to fallback
                if (n_bl_dst == n_bl_src && bl_size_dst == bl_size_src)
                {
                    mem::memcpy2D<mem::get_addr_space<self_t>, mem::get_addr_space<RHS>>((void*)data(),
                                                                                         bl_str_dst * sizeof(value_type),
                                                                                         (void*)rhs.data(),
                                                                                         bl_str_src * sizeof(value_type),
                                                                                         bl_size_src * sizeof(value_type),
                                                                                         n_bl_src);
                    return;
                }
            }
        }
    }
    // otherwise fallback to elementwise assignment
    if constexpr (mem::on_device<self_t> || mem::on_device<RHS>)
    {
        ENDA_RUNTIME_ERROR << "Error in assign_from_ndarray: Fallback to elementwise assignment not implemented for arrays/views on the GPU";
    }
    enda::for_each(shape(), [this, &rhs](auto const&... args) { (*this)(args...) = rhs(args...); });
}

template<typename Scalar>
void fill_with_scalar(Scalar const& scalar) noexcept
{
    // we make a special implementation if the array is strided in 1d or contiguous
    if constexpr (has_layout_strided_1d<self_t>)
    {
        const long L             = size();
        auto* __restrict const p = data(); // no alias possible here!
        if constexpr (has_contiguous_layout<self_t>)
        {
            for (long i = 0; i < L; ++i)
                p[i] = scalar;
        }
        else
        {
            const long stri  = indexmap().min_stride();
            const long Lstri = L * stri;
            for (long i = 0; i < Lstri; i += stri)
                p[i] = scalar;
        }
    }
    else
    {
        // no compile-time memory layout guarantees
        for (auto& x : *this)
            x = scalar;
    }
}

template<typename Scalar>
void assign_from_scalar(Scalar const& scalar) noexcept
{
    static_assert(!is_const, "Error in assign_from_ndarray: Cannot assign to a const view");
    if constexpr (Algebra != 'M')
    {
        // element-wise assignment for non-matrix algebras
        fill_with_scalar(scalar);
    }
    else
    {
        // a scalar has to be interpreted as a unit matrix for matrix algebras (the scalar in the shortest diagonal)
        // FIXME : A priori faster to put 0 everywhere and then change the diag to avoid the if.
        // FIXME : Benchmark and confirm.
        if constexpr (is_scalar_or_convertible_v<Scalar>)
            fill_with_scalar(0);
        else
            fill_with_scalar(Scalar {0 * scalar}); // FIXME : improve this
        const long imax = std::min(extent(0), extent(1));
        for (long i = 0; i < imax; ++i)
            operator()(i, i) = scalar;
    }
}
