/*
Copyright (c) 2009, Madis Kaal <mast@nomad.ee> 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the involved organizations nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

// implementation of formulas from http://williams.best.vwh.net/avform.htm
#ifndef __greatcircle_hpp__
#define __greatcircle_hpp__
#include <math.h>
#include <ctype.h>
#include <stdlib.h>

class CGreatCircle
{
public:
    struct Point {
      double lat,lon;
      
      Point()
      {
        lat=lon=0.0;
      }
      
      // define point by absolute lat and lon values
      //
      Point(double lat,double lon)
      {
        this->lat=lat;
        this->lon=lon;
      }
      
      // define point by deg/min/sec syntax
      // for example
      // 15deg 10min 15sec N
      // 15o10'15"N
      //
      Point(const char *lat,const char *lon)
      {
        this->lat=Parse(lat);
        this->lon=Parse(lon);
      }
      
      // return an equivalent point but with angular coordinates
      // given in decimal degrees
      //
      Point ToDegrees()
      {
        return Point(((double)180.0/M_PI)*this->lat,
            ((double)180.0/M_PI)*this->lon);
      }
      
      // parses string coordinates into radians
      // South and East are negative
      //
      static double Parse(const char *l)
      {
        double d;
        char *e;
        if (!l || !*l)
          return 0.0;
        d=strtod(l,&e);
        switch (*e) {
          case 'n':
          case 'N':
            break;
          case 'S':
          case 's':
            d=0-d;
            break;
          case 'E':
          case 'e':
            d=0-d;
            break;
          case 'W':
          case 'w':
            break;
          default:
            while (*e && !isdigit(*e) && e[1] )
              e++;
            d=d+strtod(e,&e)/60.0;
            switch (*e) {
              case 'n':
              case 'N':
                break;
              case 'S':
              case 's':
                d=0-d;
                break;
              case 'E':
              case 'e':
                d=0-d;
                break;
              case 'W':
              case 'w':
                break;
              default:
                while (*e && !isdigit(*e) && e[1])
                  e++;
                d=d+strtod(e,&e)/3600.0;
                switch (*e) {
                  case 'n':
                  case 'N':
                    break;
                  case 'S':
                  case 's':
                    d=0-d;
                    break;
                  case 'E':
                  case 'e':
                    d=0-d;
                    break;
                  case 'W':
                  case 'w':
                    break;
                }
            }        
        }
        return d*M_PI/(double)180.0;
      }
    };
    
    // converts angle given in degrees to equivalent angle in radians    
    //
    static double AngleRadians(double angle_degrees)
    {
      return (M_PI/(double)180.0)*angle_degrees; 
    }
    
    // converts angle given in radians to equivalent angle in degrees
    //
    static double AngleDegrees(double angle_radians)
    {
      return ((double)180/M_PI)*angle_radians; 
    }
    
    // converts a distance in nautical miles to angle in radians
    // the nautical mile is defined as 1 minute angle taken from center
    // of earth so the calculation are greatly simplified
    // 
    static double DistanceRadians(double miles)
    {
      return miles*M_PI/((double)180.0*(double)60.0);
    }
    
    // converts angle in radians to surface distance in nautical miles
    //
    static double DistanceMiles(double radians) {
      return radians*(((double)180.0*(double)60.0)/M_PI);
    }
    
    // converts distance in nautical miles into distance in kilometers
    //
    static double NM2KM(double miles) {
      return miles*(double)1.852;
    }
    
    // converts distance in kilometers into distance in nautical miles
    //
    static double KM2NM(double km)
    {
      return km/(double)1.852;
    }

    // calculates angular distance in radians between p1 and p2
    //
    static double AngularDistance(Point& p1,Point& p2)
    {
      return (double)2.0*
        asin(
          sqrt(
            pow(sin((p1.lat-p2.lat)/(double)2.0),(double)2.0) +
            cos(p1.lat)*cos(p2.lat)*pow(sin((p1.lon-p2.lon)/(double)2.0),(double)2.0)
          )
        ); 
    }
    
    // calculates a course in radians from point p1 to p2
    //
    static double Course(Point& p1,Point& p2)
    {
      // handle the case where initial point is a pole
      if (cos(p1.lat)<0.000000000000001) {
        if (p1.lat>0)
          return M_PI; 	   // starting from north pole
        else
          return M_PI*(double)2.0; // starting from south pole
      }
      return fmod(
        atan2(
          sin(p1.lon-p2.lon)*cos(p2.lat),cos(p1.lat)*sin(p2.lat)
            - sin(p1.lat)*cos(p2.lat)*cos(p1.lon-p2.lon)
        ),M_PI*(double)2.0);
    }

    // calculates coordinates of a point at given distance d from
    // origin point on a true course tc. all paramters in radians
    //
    static Point CoursePoint(Point& origin,double tc,double d)
    {
      Point p;
      p.lat=asin(sin(origin.lat)*cos(d)+cos(origin.lat)*sin(d)*cos(tc));
      double dlon=atan2(sin(tc)*sin(d)*cos(origin.lat),
            cos(d)-sin(origin.lat)*sin(p.lat));
      p.lon=fmod(origin.lon-dlon+M_PI,(double)2.0*M_PI)-M_PI;
      return p;
    }
        
    // calculates intersection angular coordinates between true courses
    // originating from two points. course angles and coordinates must
    // be in radians 
    //
    static Point InterSection(Point& p1,double crs13,Point& p2,double crs23)
    {
      double dst12=AngularDistance(p1,p2);
      double crs12,crs21;
      if (sin(p2.lon-p1.lon)<0.0) {            
        crs12=acos((sin(p2.lat)-sin(p1.lat)*cos(dst12))/(sin(dst12)*cos(p1.lat)));
        crs21=(double)2.0*M_PI-acos((sin(p1.lat)-sin(p2.lat)*cos(dst12))/(sin(dst12)*cos(p2.lat)));
      }
      else {
        crs12=(double)2.0*M_PI-acos((sin(p2.lat)-sin(p1.lat)*cos(dst12))/(sin(dst12)*cos(p1.lat)));
        crs21=acos((sin(p1.lat)-sin(p2.lat)*cos(dst12))/(sin(dst12)*cos(p2.lat)));
      }
      double ang1=fmod(crs13-crs12+M_PI,(double)2.0*M_PI)-M_PI;
      double ang2=fmod(crs21-crs23+M_PI,(double)2.0*M_PI)-M_PI;
                                   
      if (sin(ang1)==0.0 && sin(ang2)==0.0) {
        return CGreatCircle::Point(); // "infinity of intersections"
      }
      if (sin(ang1)*sin(ang2)<0) {
        return CGreatCircle::Point(); // "intersection ambiguous"
      }
      ang1=fabs(ang1);
      ang2=fabs(ang2);
      double ang3=acos(-cos(ang1)*cos(ang2)+sin(ang1)*sin(ang2)*cos(dst12));
      double dst13=atan2(sin(dst12)*sin(ang1)*sin(ang2),
          cos(ang2)+cos(ang1)*cos(ang3));
      double lat3=asin(sin(p1.lat)*cos(dst13)+cos(p1.lat)*sin(dst13)*cos(crs13));
      double dlon=atan2(sin(crs13)*sin(dst13)*cos(p1.lat),
        cos(dst13)-sin(p1.lat)*sin(lat3));
      double lon3=fmod(p1.lon-dlon+M_PI,(double)2.0*M_PI)-M_PI;
      return CGreatCircle::Point(lat3,lon3);      
    }
};

#endif


int main(int argc,char *argv[])
{
double dist;
CGreatCircle::Point lax("33deg 57min N","118deg 24min W");
CGreatCircle::Point jfk(0.709186,1.287762);
CGreatCircle::Point reo(0.74351,2.05715);
CGreatCircle::Point bke(0.782606,2.056103);
CGreatCircle::Point intersection;

  // An enroute waypoint 100nm from LAX on the 66 degree radial
  // (100nm along the GC to JFK)
  // expected result lat=0.604180,lon=2.034206
  //
  dist=CGreatCircle::DistanceRadians(100);
  intersection=CGreatCircle::CoursePoint(lax,CGreatCircle::AngleRadians(66),dist);

  // intersection of 51 degree radial out of REO and 137 degree radial
  // out of BKE
  // expected result lat=0.760473,lon=2.027876
  //
  intersection=CGreatCircle::InterSection(
  reo,CGreatCircle::AngleRadians(51),
  bke,CGreatCircle::AngleRadians(137));
  
  // take intersection coordinates in decimal degrees
  //
  intersection = intersection.ToDegrees();

  // find a distance between LAX and JFK
  // expected result 0.623585 radians
  dist=CGreatCircle::AngularDistance(lax,jfk);
  // convert the distance to nautical miles
  // expected result 2144
  dist=CGreatCircle::DistanceMiles(dist);
  // convert nautical miles to kilometers
  dist=CGreatCircle::NM2KM(dist);

  // find a true course from LAX to JFK
  // expected result 1.150035 radians
  dist=CGreatCircle::Course(lax,jfk);
  // convert to degrees
  // expected result 66 degrees
  dist=CGreatCircle::AngleDegrees(dist);
}

