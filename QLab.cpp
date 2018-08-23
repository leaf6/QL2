#include <cmath>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <set>

using namespace std;

bool analyze_line( char * buffer );

struct per_symbol_info
{
  mutable long m_max_time_gap;
  mutable long m_last_traded;
  char m_symbol[ 3 ];
  mutable int m_total_qty;
  mutable int m_max_price;
  mutable double m_weighted_av_p;

  per_symbol_info()
  {
      m_max_time_gap = 0;
      m_last_traded = 0;
      m_max_price = 0;
      m_total_qty = 0;
      m_weighted_av_p = 0;
  }

  // We'll be putting these into a set, so implement op<
  bool operator<( const per_symbol_info & rhs ) const
  {
    return( strncmp( m_symbol, rhs.m_symbol, 3 ) < 0 );
  }

  void print_result( ostream & the_stream ) const
  {
    the_stream << m_symbol[ 0 ] << m_symbol[ 1 ] << m_symbol[ 2 ] <<
      ',' << m_max_time_gap << ',' << m_total_qty << ',' <<
      floor( m_weighted_av_p ) << ',' << m_max_price << '\n';
  }
};


set<per_symbol_info> all_symbol_info;


int main( int argc, char ** argv )
{
    // Ensure they gave us the file name for the data
    if( argc < 2 )
    {
        cout << "Usage: a.out <path to data file>\n";
        exit( 1 );
    }

    try 
    {

    // Open the file and process each line as we read it
    ifstream in_file( argv[ 1 ], ifstream::in );
    if( in_file.fail() )
    {
        cout << "Could not open file input.csv, we expect to find it in the current directory.\n";
        exit( 1 );
    }

    const int RD_BUF_SZ = 30;
    char buffer[ RD_BUF_SZ ];

    int line_counter = 0;
    while( in_file.good() )
    {
        in_file.getline( buffer, RD_BUF_SZ, '\n' );

        if( in_file.eof() )
            break;

        if( in_file.fail() )
        {
	  cout << "Tried to read a line but did not get to newline character, unexpectedly long line?\n";
	  exit( 1 );
        }

	++line_counter;
        bool result = analyze_line( buffer );
	if( ! result )
        {
	  cout << "Could not successfully parse the line, unexpected format\n";
	  exit( 1 );
        }

    }
    in_file.close();

    ofstream out_file( "output.csv", ios_base::out );
    for( set<per_symbol_info>::const_iterator cit = all_symbol_info.begin();
                                       cit != all_symbol_info.end(); ++cit )
        cit->print_result( out_file );
    out_file.close();

    }
    catch( const std::exception & e )
    {
        printf( "Unexpected error encountered, terminating program: %s.\n", e.what() );
    }
    catch( ... )
    {
        printf( "Unexpected error encountered, terminating program.\n" );
    }
}


bool analyze_line( char * buffer )
{
    per_symbol_info lookup_key;

     // Pick up the timestamp
    char * next_comma = strchr( buffer, ',' );
    if( 0 == next_comma )
        return false;
    *next_comma = 0;
    unsigned long current_time_stamp = atol( buffer );
    buffer = next_comma + 1;

    // Pick up the 3-char symbol
    next_comma = strchr( buffer, ',' );
    if( 0 == next_comma )
        return false;
    *next_comma = 0;
    strncpy( lookup_key.m_symbol, buffer, 3 );

    long current_time_gap = 0;
    pair< set<per_symbol_info>::iterator, bool> itr = all_symbol_info.insert( lookup_key );
    if( 0 != itr.first->m_last_traded )
    {
        long current_gap = current_time_stamp - itr.first->m_last_traded;
        if( itr.first->m_max_time_gap < current_gap )
            itr.first->m_max_time_gap = current_gap;
    }
    itr.first->m_last_traded = current_time_stamp;

    // Pick up and process the quantity info
    buffer = next_comma + 1;
    next_comma = strchr( buffer, ',' );
    if( 0 == next_comma )
        return false;
    *next_comma = 0;
    int current_qty = atoi( buffer );

    // Pick up and process the price info
    buffer = next_comma + 1;
    int current_price = atoi( buffer );
    if( itr.first->m_max_price < current_price )
       itr.first->m_max_price = current_price;

    itr.first->m_weighted_av_p = (( itr.first->m_weighted_av_p * itr.first->m_total_qty ) + 
                          ( current_price * current_qty ) ) / ( itr.first->m_total_qty + current_qty );
    itr.first->m_total_qty += current_qty;


    return true;
}
