#include "du1matrix.hpp"
#include "du1debug.hpp"

#include <iostream>
#include <algorithm>

typedef matrix< int> my_matrix;

int cnt = 100;

void f1( my_matrix::cols_t::value_type::reference x)
{
    x = ++ cnt;
}

void f2( my_matrix::cols_t::reference r)
{
    std::for_each( r.begin(), r.end(), f1);
}

void f3( my_matrix::rows_t::value_type::reference x)
{
    std::cout << x << " ";
}

void f4( my_matrix::rows_t::reference r)
{
    std::for_each( r.begin(), r.end(), f3);
    std::cout << std::endl;
}

#ifdef SILENT_TEST
#if defined( _MSC_VER)
void myInvalidParameterHandler(const wchar_t* expression,
   const wchar_t* function, 
   const wchar_t* file, 
   unsigned int line, 
   uintptr_t pReserved)
{
   wprintf(L"Invalid parameter detected in function %s."
            L" File: %s Line: %d\n", function, file, line);
   wprintf(L"Expression: %s\n", expression);
   du_assert( 0);
}
#endif

void be_silent()
{
#if defined( _MSC_VER)
	{
		int rv1 = _set_error_mode(_REPORT_ERRMODE);
		int rv2 = _set_error_mode(_OUT_TO_STDERR);
		int rv3 = _set_error_mode(_REPORT_ERRMODE);
		//std::cerr << "Error mode switched: " << rv1 << "," << rv2 << "," << rv3 << std::endl;
	}
	{
		int rv1 = _CrtSetReportMode( _CRT_WARN, _CRTDBG_REPORT_MODE);
		int rv2 = _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE);
		int rv3 = _CrtSetReportMode( _CRT_WARN, _CRTDBG_REPORT_MODE);
		//std::cerr << "_CRT_WARN mode switched: " << rv1 << "," << rv2 << "," << rv3 << std::endl;
	}
	{
		_HFILE rv1 = _CrtSetReportFile( _CRT_WARN, _CRTDBG_REPORT_FILE );
		_HFILE rv2 = _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
		_HFILE rv3 = _CrtSetReportFile( _CRT_WARN, _CRTDBG_REPORT_FILE );
		//std::cerr << "_CRT_WARN file switched: " << rv1 << "," << rv2 << "," << rv3 << std::endl;
	}
	{
		int rv1 = _CrtSetReportMode( _CRT_ERROR, _CRTDBG_REPORT_MODE);
		int rv2 = _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE);
		int rv3 = _CrtSetReportMode( _CRT_ERROR, _CRTDBG_REPORT_MODE);
		//std::cerr << "_CRT_ERROR mode switched: " << rv1 << "," << rv2 << "," << rv3 << std::endl;
	}
	{
		_HFILE rv1 = _CrtSetReportFile( _CRT_ERROR, _CRTDBG_REPORT_FILE );
		_HFILE rv2 = _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
		_HFILE rv3 = _CrtSetReportFile( _CRT_ERROR, _CRTDBG_REPORT_FILE );
		//std::cerr << "_CRT_ERROR file switched: " << rv1 << "," << rv2 << "," << rv3 << std::endl;
	}
	{
		int rv1 = _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_REPORT_MODE);
		int rv2 = _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE);
		int rv3 = _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_REPORT_MODE);
		//std::cerr << "_CRT_ASSERT mode switched: " << rv1 << "," << rv2 << "," << rv3 << std::endl;
	}
	{
		_HFILE rv1 = _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_REPORT_FILE );
		_HFILE rv2 = _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
		_HFILE rv3 = _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_REPORT_FILE );
		//std::cerr << "_CRT_ASSERT file switched: " << rv1 << "," << rv2 << "," << rv3 << std::endl;
	}
	{
	   _invalid_parameter_handler oldHandler, newHandler;
	   newHandler = myInvalidParameterHandler;
	   oldHandler = _set_invalid_parameter_handler(newHandler);
	   //std::cerr << "invalid_parameter_handler switched: " << oldHandler << "," << newHandler << std::endl;
	}
#endif
}
#endif

struct Complex
{
    double re;
    double im;
};

int main( int, char * *)
{
#ifdef SILENT_TEST
	be_silent();
#endif

    try {

	my_matrix a( 3, 4, 0);  // matice 3 radky * 4 sloupce inicializovana nulami

	my_matrix c = a;

	my_matrix b( a);

	std::for_each( b.cols().begin(), b.cols().end(), f2);

	c = b;
	c[0][2] = b[1][1];

	std::for_each( c.rows().begin(), c.rows().end(), f4);

  // postfix operator ++
  auto it1 = c.cols()[0].begin();
  auto it2 = it1++;
  *it2 = 14;

  // const iterators
  std::for_each(c.rows().cbegin(), c.rows().cend(),
      [](my_matrix::crows_t::reference row)
      {
          std::for_each(row.cbegin(), row.cend(),
          [](my_matrix::crows_t::value_type::reference el)
          {
              std::cout << el << ' ';
          });

          std::cout << '\n';
      });

  // operator ->
  Complex zero;
  zero.re = 0.0;
  zero.im = 0.0;
  matrix<Complex> ca(2, 3, zero);
  auto it3 = ca.rows().begin();
  auto it4 = it3->begin();
  (++it4)->re = 2.0;

  // const matrix
  const my_matrix d = c;

  my_matrix::cols_t::value_type::iterator nit;
  my_matrix::cols_t::value_type::const_iterator cit = nit;
  cit = nit;

  // nit = cit; // compile-time error

  my_matrix::row_t r = a.rows()[0];
  my_matrix::crow_t cr = r;

  my_matrix::rows_t::iterator nnit;
  my_matrix::rows_t::const_iterator ccit = nnit;

  my_matrix::rows_t qq1 = a.rows();
  my_matrix::crows_t qq2 = qq1;
  // qq1 = qq2; // compile-time error


#ifndef _MSC_VER // Not sure if MSVC 2012 supports range based for
  // new foreach
  for (auto col : ca.ccols())
  {
      for (auto el : col)
      {
          std::cout << el.re << " + " << el.im << "i   ";
      }

      std::cout << '\n';
  }

  for (auto col : d.cols())
  {
      for (auto el : col)
      {
          std::cout << el << " ";
      }

      std::cout << '\n';
  }
#endif

	my_matrix::cols_t::iterator rowit;

	rowit->end();

    }
    catch ( const std::exception & e)
    {
	std::cout << e.what() << std::endl;
    }

    return 0;
}
