#include "json.h"
#include "string_formatter.h"
#include "units.h"
#include "translations.h"

namespace units
{
template<>
void volume::serialize( JsonOut &jsout ) const
{
    if( value_ % 1000 == 0 ) {
        jsout.write( string_format( "%d L", value_ / 1000 ) );
    } else {
        jsout.write( string_format( "%d ml", value_ ) );
    }
}

template<>
void mass::serialize( JsonOut &jsout ) const
{
    if( value_ % 1000000 == 0 ) {
        jsout.write( string_format( "%d kg", value_ / 1000000 ) );
    } else if( value_ % 1000 == 0 ) {
        jsout.write( string_format( "%d g", value_ / 1000 ) );
    } else {
        jsout.write( string_format( "%d mg", value_ ) );
    }
}

std::ostream &operator<<( std::ostream &o, mass_in_milligram_tag )
{
    return o << "mg";
}

std::ostream &operator<<( std::ostream &o, volume_in_milliliter_tag )
{
    return o << "ml";
}

std::ostream &operator<<( std::ostream &o, energy_in_millijoule_tag )
{
    return o << "mJ";
}

std::ostream &operator<<( std::ostream &o, money_in_cent_tag )
{
    return o << "cent";
}

std::ostream &operator<<( std::ostream &o, temperature_in_millidegree_celsius_tag )
{
    return o << "mC";
}

std::string display( const units::energy v )
{
    const int kj = units::to_kilojoule( v );
    const int j = units::to_joule( v );
    // at least 1 kJ and there is no fraction
    if( kj >= 1 && float( j ) / kj == 1000 ) {
        return std::to_string( kj ) + ' ' + pgettext( "energy unit: kilojoule", "kJ" );
    }
    const int mj = units::to_millijoule( v );
    // at least 1 J and there is no fraction
    if( j >= 1 && float( mj ) / j  == 1000 ) {
        return std::to_string( j ) + ' ' + pgettext( "energy unit: joule", "J" );
    }
    return std::to_string( mj ) + ' ' + pgettext( "energy unit: millijoule", "mJ" );
}
} // namespace units
