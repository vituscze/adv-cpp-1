// Author: Vít Šefl
// Advanced C++, 1st assignment
// NPRG051 2012/2013
#ifndef DU1_MATRIX_HPP
#define DU1_MATRIX_HPP

#include <iterator>
#include <utility>
#include <vector>

#include "du1debug.hpp"

//   matrix class template
//   =====================
//
//   Overview
//   --------
//
//   Matrix is a fixed-width two dimensional container supporting element access
// (operator[]), row-wise and column-wise view (rows(), crows(), cols(),
// ccols()).
//
//   Rows and columns views behave as proxy containers, supporting element
// access and iterator access through the member functions begin() and end()
// (cbegin() and cend() being their respective const versions).
//
//   The elements of rows or columns view (accessed either directly via
// element access operator or indirectly via iterator) form another proxy
// container representing a specific row or column.
//
//    These rows and columns can also be manipulated either direclty via
// element access operator (which gives a reference to the actual data stored
// inside the matrix) or indirectly via iterator (again, through member
// functions begin() and end() and their const variants).
//
//   At the moment, all exposed iterators are forward iterators only. All
// iterators also support iterator to const iterator conversion.
//
//   Example usage
//   -------------
//
//   matrix<int> m(...);
//
//   for (auto it1 = m.cols().begin(), end1 = m.cols().end(); it1 != end1; ++it1)
//   {
//       for (auto it2 = it1->begin(), end2 = it1->end(); it2 != end2; ++it2)
//       {
//           f(*it2);
//       }
//   }
//
//   Implementation details
//   ----------------------
//
//   The following diagram represents the structure of column non-const and
// const row and rows views and their respective iterators. Note that
// the implementation of column views is symmetric, with the exception of
// matrix::operator[].
//
//  ///////////////////////////////////////////////////////////////////////
//  //                                                                   //
//  //                MATRIX PROXY AND ITERATOR STRUCTURE                //
//  //                -----------------------------------                //
//  //                                                                   //
//  //                                                                   //
//  //   op[] (row_t only)  +--------+                                   //
//  //    ------------------| matrix |                                   //
//  //   /                  +--------+                                   //
//  //  |            rows() /        \ crows()                           //
//  //  |                  /          \                                  //
//  //  |  op[]  +--------+            +---------+     op[]              //
//  //  | -------| rows_t |-----       | crows_t |--------------         //
//  //  |/       +--------+     \      +---------+              \        //
//  //  |            |           |          |                    |       //
//  //  |    begin() |  cbegin() |          | begin(), cbegin()  |       //
//  //  |      end() |    cend() |          | end(), cend()      |       //
//  //  |            |           |          |                    |       //
//  //  |   +-----------------+   \ +------------------+         |       //
//  //  |   | rows_t_iterator |    -| crows_t_iterator |         |       //
//  //  |   +-----------------+     +------------------+         |       //
//  //  |            |                      |                    |       //
//  //  |            | op*                  | op*                |       //
//  //  |            | op->                 | op->               |       //
//  //  |            |                      |                    |       //
//  //   \       +-------+              +--------+              /        //
//  //    -------| row_t |------        | crow_t |--------------         //
//  //   /       +-------+      \       +--------+              \        //
//  //  |            |           |          |                    |       //
//  //  |    begin() |  cbegin() |          | begin(), cbegin()  |       //
//  //  |      end() |    cend() |          | end(), cend()      |       //
//  //  |            |           |          |                    |       //
//  //  |    +----------------+   \ +-----------------+          | op[]  //
//  //  |    | row_t_iterator |    -| crow_t_iterator |          |       //
//  //  |    +----------------+     +-----------------+          |       //
//  //  |            |                      |                    |       //
//  //  | op[]       | op*                  | op*                |       //
//  //  |            | op->                 | op->               |       //
//  //  |            |                      |                    |       //
//  //   \         +---+               +---------+              /        //
//  //    ---------| T |               | const T |--------------         //
//  //             +---+               +---------+                       //
//  //                                                                   //
//  ///////////////////////////////////////////////////////////////////////
//
//   Container and iterator inner types are defined in the following
// structures:
//
//                           struct | corresponding type
//  --------------------------------+----------------------------------
//            element_iterator_base | col_t_iterator row_t_iterator
//      const_element_iterator_base | ccol_t_iterator crow_t_iterator
//                   c_element_base | col_t
//             const_c_element_base | ccol_t
//                   r_element_base | row_t
//             const_r_element_base | crow_t
//        col_element_iterator_base | cols_t_iterator
//  const_col_element_iterator_base | ccols_t_iterator
//        row_element_iterator_base | rows_t_iterator
//  const_row_element_iterator_base | crows_t_iterator
//                 col_element_base | cols_t
//           const_col_element_base | ccols_t
//                 row_element_base | rows_t
//           const_row_element_base | crows_t
//
//
//   This design helps to reduce code duplication by shifting the const vs.
// non-const difference into a small struct. All other proxies and their
// iterators are class templates that inherit those type definitions from their
// type parameter. So, for example, col_t and ccol_t are actually typedefs for
// the following classes:
//
//   typedef col_t_base<      c_element_base> col_t;
//   typedef col_t_base<const_c_element_base> ccol_t;
//
//   cols_t, ccols_t, rows_t and crows_t only contain a (const) pointer to
// the matrix. begin() returns an iterator with zero offset, end() with the
// offset being the number of columns (or rows, respectively). operator[]
// returns col_t, row_t (and their const variants respectively) directly
// focusing specific column or row.
//
//   cols_t_iterator, ccols_t_iterator, rows_t_iterator and crows_t_iterator
// contain col_t, ccol_t, row_t and crow_t respectively. Since current row or
// column have no direct representation inside the matrix, operators * and ->
// return a (non-constant) reference and pointer to an object representing
// current line inside the iterator.
//
//   Digression:
//
//   Since row_t, crow_t, col_t and ccol_t offer no public
// operations that mutate its state, the field inside the iterators mentioned
// above is declared mutable. This allows to keep logically constant operations
// (such as operator * and ->) const while still allowing us to return a
// non-constant reference or pointer.
//
//   Other options include marking * and -> as non-const or forcing them
// to return a constant reference or pointer. All these options are viable,
// but the last one might force certain typedefs which should not be const, to
// become const. For simplicity and consistency, a mutable field is used.
//
//   col_t, ccol_t, row_t and crow_t contain a pointer to the matrix and the
// number of current line. begin() and end() return an iterator capable of
// directly accessing the values stored inside the matrix. operator[] also gives
// direct access to the matrix.
//
//   col_t_iterator, ccol_t_iterator, row_t_iterator, crow_t_iterator contain
// a pointer to the matrix, the number of the current line and the current
// offset.
//
template <typename T>
class matrix
{
    typedef matrix<T>      self;

public:
    typedef T              value_type;
    typedef T&             reference;
    typedef T*             pointer;
    typedef const T&       const_reference;
    typedef const T*       const_pointer;
    typedef std::ptrdiff_t difference_type;
    typedef std::size_t    size_type;

    // Constructors.
    matrix()
        : data_()
        , rows_()
        , cols_()
    { }

    matrix(size_type rows, size_type cols, const value_type& def)
        : data_(rows * cols, def)
        , rows_(rows)
        , cols_(cols)
    { }

    matrix(const self&) = default;
    matrix(self&&) = default;

    // Assignment.
    self& operator=(const self&) = default;
    self& operator=(self&&) = default;

    // Forward declaration of helper class templates.
    template <typename Base>
    class col_t_iterator_base;

    template <typename Base>
    class col_t_base;

    template <typename Base>
    class cols_t_iterator_base;

    template <typename Base>
    class cols_t_base;

    template <typename Base>
    class row_t_iterator_base;

    template <typename Base>
    class row_t_base;

    template <typename Base>
    class rows_t_iterator_base;

    template <typename Base>
    class rows_t_base;

    // Structs containing proxy container and iterator type definitions.
private:
    struct element_iterator_base
    {
        typedef T  value_type;
        typedef T& reference;
        typedef T* pointer;

        typedef self* matrix_pointer;
    };

    struct const_element_iterator_base
    {
        typedef const T  value_type;
        typedef const T& reference;
        typedef const T* pointer;

        typedef const self* matrix_pointer;
    };

public:
    typedef col_t_iterator_base<element_iterator_base>       col_t_iterator;
    typedef col_t_iterator_base<const_element_iterator_base> ccol_t_iterator;

    typedef row_t_iterator_base<element_iterator_base>       row_t_iterator;
    typedef row_t_iterator_base<const_element_iterator_base> crow_t_iterator;

private:
    struct c_element_base
    {
        typedef T        value_type;
        typedef T&       reference;
        typedef T*       pointer;
        typedef const T& const_reference;
        typedef const T* const_pointer;

        typedef col_t_iterator  iterator;
        typedef ccol_t_iterator const_iterator;

        typedef self* matrix_pointer;
    };

    struct const_c_element_base
    {
        typedef const T  value_type;
        typedef const T& reference;
        typedef const T* pointer;
        typedef const T& const_reference;
        typedef const T* const_pointer;

        typedef ccol_t_iterator iterator;
        typedef ccol_t_iterator const_iterator;

        typedef const self* matrix_pointer;
    };

    struct r_element_base
    {
        typedef T        value_type;
        typedef T&       reference;
        typedef T*       pointer;
        typedef const T& const_reference;
        typedef const T* const_pointer;

        typedef row_t_iterator  iterator;
        typedef crow_t_iterator const_iterator;

        typedef self* matrix_pointer;
    };

    struct const_r_element_base
    {
        typedef const T  value_type;
        typedef const T& reference;
        typedef const T* pointer;
        typedef const T& const_reference;
        typedef const T* const_pointer;

        typedef crow_t_iterator iterator;
        typedef crow_t_iterator const_iterator;

        typedef const self* matrix_pointer;
    };

public:
    typedef col_t_base<c_element_base>       col_t;
    typedef col_t_base<const_c_element_base> ccol_t;

    typedef row_t_base<r_element_base>       row_t;
    typedef row_t_base<const_r_element_base> crow_t;

private:
    struct col_element_iterator_base
    {
        typedef col_t  value_type;
        typedef col_t& reference;
        typedef col_t* pointer;

        typedef col_t line_type;
        typedef self* matrix_pointer;
    };

    struct const_col_element_iterator_base
    {
        typedef ccol_t  value_type;
        typedef ccol_t& reference;
        typedef ccol_t* pointer;

        typedef ccol_t      line_type;
        typedef const self* matrix_pointer;
    };

    struct row_element_iterator_base
    {
        typedef row_t  value_type;
        typedef row_t& reference;
        typedef row_t* pointer;

        typedef row_t line_type;
        typedef self* matrix_pointer;
    };

    struct const_row_element_iterator_base
    {
        typedef crow_t  value_type;
        typedef crow_t& reference;
        typedef crow_t* pointer;

        typedef crow_t      line_type;
        typedef const self* matrix_pointer;
    };

public:
    typedef cols_t_iterator_base<col_element_iterator_base>       cols_t_iterator;
    typedef cols_t_iterator_base<const_col_element_iterator_base> ccols_t_iterator;

    typedef rows_t_iterator_base<row_element_iterator_base>       rows_t_iterator;
    typedef rows_t_iterator_base<const_row_element_iterator_base> crows_t_iterator;

private:
    struct col_element_base
    {
        typedef col_t   value_type;
        typedef col_t&  reference;
        typedef col_t*  pointer;
        typedef ccol_t& const_reference;
        typedef ccol_t* const_pointer;

        typedef cols_t_iterator  iterator;
        typedef ccols_t_iterator const_iterator;

        typedef self* matrix_pointer;
    };

    struct const_col_element_base
    {
        typedef ccol_t  value_type;
        typedef ccol_t& reference;
        typedef ccol_t* pointer;
        typedef ccol_t& const_reference;
        typedef ccol_t* const_pointer;

        typedef ccols_t_iterator iterator;
        typedef ccols_t_iterator const_iterator;

        typedef const self* matrix_pointer;
    };

    struct row_element_base
    {
        typedef row_t   value_type;
        typedef row_t&  reference;
        typedef row_t*  pointer;
        typedef crow_t& const_reference;
        typedef crow_t* const_pointer;

        typedef rows_t_iterator  iterator;
        typedef crows_t_iterator const_iterator;

        typedef self* matrix_pointer;
    };

    struct const_row_element_base
    {
        typedef crow_t  value_type;
        typedef crow_t& reference;
        typedef crow_t* pointer;
        typedef crow_t& const_reference;
        typedef crow_t* const_pointer;

        typedef crows_t_iterator iterator;
        typedef crows_t_iterator const_iterator;

        typedef const self* matrix_pointer;
    };

public:
    typedef cols_t_base<col_element_base>       cols_t;
    typedef cols_t_base<const_col_element_base> ccols_t;

    typedef rows_t_base<row_element_base>       rows_t;
    typedef rows_t_base<const_row_element_base> crows_t;

    // Row and column proxies and iterator helper class templates.
    template <typename Base>
    class col_t_iterator_base : public Base
    {
        friend col_t;
        friend ccol_t;

        // Friend declaration to allow conversion operations.
        template <typename>
        friend class col_t_iterator_base;

        using typename Base::matrix_pointer;

    public:
        using typename Base::value_type;
        using typename Base::reference;
        using typename Base::pointer;

        typedef std::ptrdiff_t            difference_type;
        typedef std::forward_iterator_tag iterator_category;

        col_t_iterator_base()
            : matrix_(nullptr)
            , cur_col_()
            , offset_()
        { }

        // Copy and conversion constructor.
        template <typename U>
        col_t_iterator_base(const col_t_iterator_base<U>& other)
            : matrix_(other.matrix_)
            , cur_col_(other.cur_col_)
            , offset_(other.offset_)
        { }

        // Copy and conversion assignment operator.
        template <typename U>
        col_t_iterator_base& operator=(const col_t_iterator_base<U>& other)
        {
            matrix_ = other.matrix_;
            cur_col_ = other.cur_col_;
            offset_ = other.offset_;

            return *this;
        }

        bool operator==(const col_t_iterator_base& other) const
        {
            return matrix_ == other.matrix_
                && cur_col_ == other.cur_col_
                && offset_ == other.offset_;
        }

        bool operator!=(const col_t_iterator_base& other) const
        {
            return !(*this == other);
        }

        reference operator*() const
        {
            du_assert(matrix_ && offset_  >= 0 && offset_  < matrix_->rows_
                              && cur_col_ >= 0 && cur_col_ < matrix_->cols_);


            return matrix_->data_[cur_col_ + matrix_->cols_ * offset_];
        }

        pointer operator->() const
        {
            return &**this;
        }

        col_t_iterator_base& operator++()
        {
            du_assert(matrix_ && offset_  <= matrix_->rows_
                              && cur_col_ >= 0 && cur_col_ < matrix_->cols_);


            ++offset_;
            return *this;
        }

        col_t_iterator_base operator++(int)
        {
            col_t_iterator_base copy(*this);
            ++*this;
            return copy;
        }

    private:
        col_t_iterator_base(matrix_pointer matrix, difference_type cur_col, difference_type offset)
            : matrix_(matrix)
            , cur_col_(cur_col)
            , offset_(offset)
        { }

        matrix_pointer  matrix_;
        difference_type cur_col_;
        difference_type offset_;
    };

    template <typename Base>
    class row_t_iterator_base : public Base
    {
        friend row_t;
        friend crow_t;

        // Friend declaration to allow conversion operations.
        template <typename>
        friend class row_t_iterator_base;

        using typename Base::matrix_pointer;

    public:
        using typename Base::value_type;
        using typename Base::reference;
        using typename Base::pointer;

        typedef std::ptrdiff_t            difference_type;
        typedef std::forward_iterator_tag iterator_category;

        row_t_iterator_base()
            : matrix_(nullptr)
            , cur_row_()
            , offset_()
        { }

        // Copy and conversion constructor.
        template <typename U>
        row_t_iterator_base(const row_t_iterator_base<U>& other)
            : matrix_(other.matrix_)
            , cur_row_(other.cur_row_)
            , offset_(other.offset_)
        { }

        // Copy and conversion assignment operator.
        template <typename U>
        row_t_iterator_base& operator=(const row_t_iterator_base<U>& other)
        {
            matrix_ = other.matrix_;
            cur_row_ = other.cur_row_;
            offset_ = other.offset_;

            return *this;
        }

        bool operator==(const row_t_iterator_base& other) const
        {
            return matrix_ == other.matrix_
                && cur_row_ == other.cur_row_
                && offset_ == other.offset_;
        }

        bool operator!=(const row_t_iterator_base& other) const
        {
            return !(*this == other);
        }

        reference operator*() const
        {
            du_assert(matrix_ && offset_  >= 0 && offset_  < matrix_->cols_
                              && cur_row_ >= 0 && cur_row_ < matrix_->rows_);

            return matrix_->data_[cur_row_ * matrix_->cols_ + offset_];
        }

        pointer operator->() const
        {
            return &**this;
        }

        row_t_iterator_base& operator++()
        {
            du_assert(matrix_ && offset_  <= matrix_->cols_
                              && cur_row_ >= 0 && cur_row_ < matrix_->rows_);

            ++offset_;
            return *this;
        }

        row_t_iterator_base operator++(int)
        {
            row_t_iterator_base copy(*this);
            ++*this;
            return copy;
        }

    private:
        row_t_iterator_base(matrix_pointer matrix, difference_type cur_row, difference_type offset)
            : matrix_(matrix)
            , cur_row_(cur_row)
            , offset_(offset)
        { }

        matrix_pointer  matrix_;
        difference_type cur_row_;
        difference_type offset_;
    };

    template <typename Base>
    class col_t_base : public Base
    {
        friend cols_t;
        friend cols_t_iterator;

        friend ccols_t;
        friend ccols_t_iterator;

        // Friend declaration to allow conversion operations.
        template <typename>
        friend class col_t_base;

        using typename Base::matrix_pointer;

    public:
        using typename Base::value_type;
        using typename Base::reference;
        using typename Base::pointer;
        using typename Base::const_reference;
        using typename Base::const_pointer;

        typedef std::ptrdiff_t difference_type;
        typedef std::size_t    size_type;

        using typename Base::iterator;
        using typename Base::const_iterator;

        // Copy and conversion constructor.
        template <typename U>
        col_t_base(const col_t_base<U>& other)
            : matrix_(other.matrix_)
            , cur_col_(other.cur_col_)
        { }

        iterator begin() const
        {
            return iterator(matrix_, cur_col_, 0);
        }

        const_iterator cbegin() const
        {
            return const_iterator(matrix_, cur_col_, 0);
        }

        iterator end() const
        {
            return iterator(matrix_, cur_col_, matrix_->rows_);
        }

        const_iterator cend() const
        {
            return const_iterator(matrix_, cur_col_, matrix_->rows_);
        }

        size_type size() const
        {
            return matrix_->rows_;
        }

        reference operator[](size_type n) const
        {
            du_assert(n < matrix_->rows_ && cur_col_ >= 0
                                         && cur_col_ < matrix_->cols_);

            return matrix_->data_[cur_col_ + matrix_->cols_ * n];
        }

    private:
        col_t_base(matrix_pointer matrix, difference_type cur_col)
            : matrix_(matrix)
            , cur_col_(cur_col)
        { }

        col_t_base()
            : matrix_(nullptr)
            , cur_col_()
        { }

        matrix_pointer  matrix_;
        difference_type cur_col_;
    };

    template <typename Base>
    class row_t_base : public Base
    {
        friend rows_t;
        friend rows_t_iterator;

        friend crows_t;
        friend crows_t_iterator;

        // Friend declaration to allow conversion operations.
        template <typename>
        friend class row_t_base;

        using typename Base::matrix_pointer;

    public:
        using typename Base::value_type;
        using typename Base::reference;
        using typename Base::pointer;
        using typename Base::const_reference;
        using typename Base::const_pointer;

        typedef std::ptrdiff_t difference_type;
        typedef std::size_t    size_type;

        using typename Base::iterator;
        using typename Base::const_iterator;

        // Copy and conversion constructor.
        template <typename U>
        row_t_base(const row_t_base<U>& other)
            : matrix_(other.matrix_)
            , cur_row_(other.cur_row_)
        { }

        iterator begin() const
        {
            return iterator(matrix_, cur_row_, 0);
        }

        const_iterator cbegin() const
        {
            return const_iterator(matrix_, cur_row_, 0);
        }

        iterator end() const
        {
            return iterator(matrix_, cur_row_, matrix_->cols_);
        }

        const_iterator cend() const
        {
            return const_iterator(matrix_, cur_row_, matrix_->cols_);
        }

        size_type size() const
        {
            return matrix_->cols_;
        }

        reference operator[](size_type n) const
        {
            du_assert(n < matrix_->cols_ && cur_row_ >= 0
                                         && cur_row_ < matrix_->rows_);

            return matrix_->data_[cur_row_ * matrix_->cols_ + n];
        }

    private:
        row_t_base(matrix_pointer matrix, difference_type cur_row)
            : matrix_(matrix)
            , cur_row_(cur_row)
        { }

        row_t_base()
            : matrix_(nullptr)
            , cur_row_()
        { }

        matrix_pointer  matrix_;
        difference_type cur_row_;
    };

    template <typename Base>
    class cols_t_iterator_base : public Base
    {
        friend cols_t;
        friend ccols_t;

        // Friend declaration to allow conversion operations.
        template <typename>
        friend class cols_t_iterator_base;

        using typename Base::line_type;
        using typename Base::matrix_pointer;

    public:
        using typename Base::value_type;
        using typename Base::reference;
        using typename Base::pointer;

        typedef std::ptrdiff_t            difference_type;
        typedef std::forward_iterator_tag iterator_category;

        cols_t_iterator_base() = default;

        // Copy and conversion constructor.
        template <typename U>
        cols_t_iterator_base(const cols_t_iterator_base<U>& other)
            : it_(other.it_)
        { }

        // Copy and conversion assignment operator.
        template <typename U>
        cols_t_iterator_base& operator=(const cols_t_iterator_base<U>& other)
        {
            it_ = other.it_;

            return *this;
        }

        bool operator==(const cols_t_iterator_base& other) const
        {
            return it_.matrix_ == other.it_.matrix_
                && it_.cur_col_ == other.it_.cur_col_;
        }

        bool operator!=(const cols_t_iterator_base& other) const
        {
            return !(*this == other);
        }

        reference operator*() const
        {
            du_assert(it_.matrix_ && it_.cur_col_ >= 0
                                  && it_.cur_col_ < it_.matrix_->cols_);

            return it_;
        }

        pointer operator->() const
        {
            return &**this;
        }

        cols_t_iterator_base& operator++()
        {
            du_assert(it_.matrix_ && it_.cur_col_ <= it_.matrix_->cols_);

            ++it_.cur_col_;
            return *this;
        }

        cols_t_iterator_base operator++(int)
        {
            cols_t_iterator_base copy(*this);
            ++*this;
            return copy;
        }


    private:
        cols_t_iterator_base(matrix_pointer matrix, difference_type offset)
            : it_(matrix, offset)
        { }

        // See 'Implementation details'.
        mutable line_type it_;
    };

    template <typename Base>
    class rows_t_iterator_base : public Base
    {
        friend rows_t;
        friend crows_t;

        // Friend declaration to allow conversion operations.
        template <typename>
        friend class rows_t_iterator_base;

        using typename Base::line_type;
        using typename Base::matrix_pointer;

    public:
        using typename Base::value_type;
        using typename Base::reference;
        using typename Base::pointer;

        typedef std::ptrdiff_t            difference_type;
        typedef std::forward_iterator_tag iterator_category;

        rows_t_iterator_base() = default;

        // Copy and conversion constructor.
        template <typename U>
        rows_t_iterator_base(const rows_t_iterator_base<U>& other)
            : it_(other.it_)
        { }

        // Copy and conversion assignment operator.
        template <typename U>
        rows_t_iterator_base& operator=(const rows_t_iterator_base<U>& other)
        {
            it_ = other.it_;

            return *this;
        }

        bool operator==(const rows_t_iterator_base& other) const
        {
            return it_.matrix_ == other.it_.matrix_
                && it_.cur_row_ == other.it_.cur_row_;
        }

        bool operator!=(const rows_t_iterator_base& other) const
        {
            return !(*this == other);
        }

        reference operator*() const
        {
            du_assert(it_.matrix_ && it_.cur_row_ >= 0
                                  && it_.cur_row_ < it_.matrix_->rows_);

            return it_;
        }

        pointer operator->() const
        {
            return &**this;
        }

        rows_t_iterator_base& operator++()
        {
            du_assert(it_.matrix_ && it_.cur_row_ <= it_.matrix_->rows_);

            ++it_.cur_row_;
            return *this;
        }

        rows_t_iterator_base operator++(int)
        {
            rows_t_iterator_base copy(*this);
            ++*this;
            return copy;
        }


    private:
        rows_t_iterator_base(matrix_pointer matrix, difference_type offset)
            : it_(matrix, offset)
        { }

        // See 'Implementation details'.
        mutable line_type it_;
    };

    template <typename Base>
    class cols_t_base : public Base
    {
        friend self;

        // Friend declaration to allow conversion operations.
        template <typename>
        friend class cols_t_base;

        using typename Base::matrix_pointer;

    public:
        using typename Base::value_type;
        using typename Base::reference;
        using typename Base::pointer;
        using typename Base::const_reference;
        using typename Base::const_pointer;

        typedef std::ptrdiff_t difference_type;
        typedef std::size_t    size_type;

        using typename Base::iterator;
        using typename Base::const_iterator;

        // Copy and conversion constructor.
        template <typename U>
        cols_t_base(const cols_t_base<U>& other)
            : matrix_(other.matrix_)
        { }

        iterator begin() const
        {
            return iterator(matrix_, 0);
        }

        const_iterator cbegin() const
        {
            return const_iterator(matrix_, 0);
        }

        iterator end() const
        {
            return iterator(matrix_, matrix_->cols_);
        }

        const_iterator cend() const
        {
            return const_iterator(matrix_, matrix_->cols_);
        }

        size_type size() const
        {
            return matrix_->cols_;
        }

        value_type operator[](size_type n) const
        {
            du_assert(n < matrix_->cols_);

            return value_type(matrix_, n);
        }

    private:
        cols_t_base(matrix_pointer matrix)
            : matrix_(matrix)
        { }

        matrix_pointer matrix_;
    };

    template <typename Base>
    class rows_t_base : public Base
    {
        friend self;

        // Friend declaration to allow conversion operations.
        template <typename>
        friend class rows_t_base;

        using typename Base::matrix_pointer;

    public:
        using typename Base::value_type;
        using typename Base::reference;
        using typename Base::pointer;
        using typename Base::const_reference;
        using typename Base::const_pointer;

        typedef std::ptrdiff_t difference_type;
        typedef std::size_t    size_type;

        using typename Base::iterator;
        using typename Base::const_iterator;

        // Copy and conversion constructor.
        template <typename U>
        rows_t_base(const rows_t_base<U>& other)
            : matrix_(other.matrix_)
        { }

        iterator begin() const
        {
            return iterator(matrix_, 0);
        }

        const_iterator cbegin() const
        {
            return const_iterator(matrix_, 0);
        }

        iterator end() const
        {
            return iterator(matrix_, matrix_->rows_);
        }

        const_iterator cend() const
        {
            return const_iterator(matrix_, matrix_->rows_);
        }

        size_type size() const
        {
            return matrix_->rows_;
        }

        value_type operator[](size_type n) const
        {
            du_assert(n < matrix_->rows_);

            return value_type(matrix_, n);
        }

    private:
        rows_t_base(matrix_pointer matrix)
            : matrix_(matrix)
        { }

        matrix_pointer matrix_;
    };

    // Column views.
    cols_t cols()
    {
        return cols_t(this);
    }

    ccols_t cols() const
    {
        return ccols_t(this);
    }

    ccols_t ccols() const
    {
        return cols();
    }

    // Row views.
    rows_t rows()
    {
        return rows_t(this);
    }

    crows_t rows() const
    {
        return crows_t(this);
    }

    crows_t crows() const
    {
        return rows();
    }

    // Element access via proxy container.
    row_t operator[](size_type n)
    {
        return rows()[n];
    }

    crow_t operator[](size_type n) const
    {
        return rows()[n];
    }

private:
    std::vector<value_type> data_;
    size_type rows_;
    size_type cols_;
};

#endif // DU1_MATRIX_HPP
