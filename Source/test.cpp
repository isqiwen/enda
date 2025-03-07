/*
 * @tparam ValueType: This is the type of the elements stored in the array. Most of the time, this will be a scalar type like an int, double or
 * std::complex<double>, but it can also be a more complex type like a custom class
 * @tparam Rank: Integer specifying the number of dimensions of the array. This is a compile-time constant.
 * @tparam LayoutPolicy: The layout policy specifies how the array views the memory it uses and how it accesses its
 * elements. It provides a mapping from multi-dimensional to linear indices and vice versa.
 * @tparam Algebra: The algebra specifies how an array behaves when it is used in an expression. Possible values are 'A' (array), 'M' (matrix) and 'V'
 * (vector).
 * @tparam ContainerPolicy: The container policy specifies how and where the data is stored. It is responsible for
 * allocating/deallocating the memory.
 */
template<typename ValueType, int Rank, typename LayoutPolicy, char Algebra, typename ContainerPolicy>
class Array;
