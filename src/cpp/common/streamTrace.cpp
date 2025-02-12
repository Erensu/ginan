
// #pragma GCC optimize ("O0")

#include "streamTrace.hpp"
#include <stdarg.h>
#include <ctype.h>
#include <boost/iostreams/stream.hpp>
#ifndef WIN32
#	include <dirent.h>
#	include <time.h>
#	include <sys/time.h>
#	include <sys/stat.h>
#	include <sys/types.h>
#endif

#include <boost/format.hpp>

#include "observations.hpp"
#include "navigation.hpp"
#include "constants.hpp"
#include "common.hpp"
#include "gTime.hpp"

boost::iostreams::stream< boost::iostreams::null_sink > nullStream( ( boost::iostreams::null_sink() ) );


void tracematpde(
	int				level,          ///< []
	std::ostream&	stream,         ///< []
	MatrixXd&		mat,            ///< []
	int				width,          ///< []
	int				precision)		///< []
{
	if (level > trace_level)
		return;

	stream	<< std::endl
			<< std::fixed
			<< std::setw(width)
			<< std::setprecision(precision)
			<< std::endl
			<< mat
			<< std::endl;
}

void tracematpde(
	int				level,      	///< []
	std::ostream&	stream,         ///< []
	VectorXd&		vec,            ///< []
	int				width,          ///< []
	int				precision)      ///< []
{
	if (level > trace_level)
		return;

	stream	<< std::fixed
			<< std::setw(width)
			<< std::setprecision(precision)
			<< vec//.transpose()
			<< std::endl;
}

void tracematpde(
	int				level,     		///< []
	std::ostream&	stream,         ///< []
	MatrixXd*		mat,            ///< []
	int				width,          ///< []
	int				precision)      ///< []
{
	if (level > trace_level)
		return;

	stream	<< std::fixed
			<< std::setw(width)
			<< std::setprecision(precision)
			<< *mat
			<< std::endl;
}

void tracematpde(
	int				level,      	///< []
	std::ostream&	stream,         ///< []
	VectorXd*		vec,            ///< []
	int				width,          ///< []
	int 			precision)      ///< []
{
	if (level > trace_level)
		return;

	stream	<< std::fixed
			<< std::setw(width)
			<< std::setprecision(precision)
			<< vec->transpose()
			<< std::endl;
}

/* fatal error ---------------------------------------------------------------*/
[[deprecated]]
void fatalerr(const char *format, ...)
{
	va_list ap;
	va_start(ap,format); vfprintf(stderr,format,ap); va_end(ap);
	exit(-9);
}

/* print matrix ----------------------------------------------------------------
* print matrix to stdout
* args   : double *A        I   matrix A (n x m)
*          int    n,m       I   number of rows and columns of A
*          int    p,q       I   total columns, columns under decimal point
*         (FILE  *fp        I   output file pointer)
* return : none
* notes  : matirix stored by column-major order (fortran convention)
*-----------------------------------------------------------------------------*/
[[deprecated]]
void matfprint(const double A[], int n, int m, int p, int q, FILE *fp)
{
	int i,j;

	for (i=0;i<n;i++) {
		for (j=0;j<m;j++) fprintf(fp," %*.*f",p,q,A[i+j*n]);
		fprintf(fp,"\n");
	}
}

/* debug trace functions -----------------------------------------------------*/

int trace_level = 0;       /* level of trace */

void tracelevel(int level)
{
	trace_level = level;
}

void traceFormatedFloat(Trace& trace, double val, string formatStr)
{
	/* If someone knows how to make C++ print with just one digit as exponent... */
	int		exponent	= 0;
	double	base		= 0;

	if (val != 0)
	{
		exponent	= (int)floor(log10(fabs(val)));
		base		= val * pow(10, -1 * exponent);
	}
	tracepdeex(0, trace, formatStr.c_str(), base, exponent);
}

void tracepdeex(int level, FILE *fppde, const char *format, ...)
{
	va_list ap;

	if (!fppde||level>trace_level)
		return;
	
	va_start(ap,format); 
		vfprintf(fppde,format,ap);
	va_end(ap);
	
	fflush(fppde);
}

[[deprecated]]
void tracematpde(int level, FILE *fppde, const double *A, int n, int m, int p, int q)
{
	if (!fppde||level>trace_level) return;
	matfprint(A,n,m,p,q,fppde); fflush(fppde);
}
