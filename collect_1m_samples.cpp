#include <iostream>
#include <fstream>
#include <string>


using namespace std;

int main()
{



        ifstream pmod1;
	ofstream dataFile;

        string number;

        //pmod1.open("/sys/bus/iio/devices/iio\:device1/in_proximity_raw");

	dataFile.open("/home/alarm/pmodSamples.txt"); //path to data file once it is placed on board

        for( int i=0;i<1000000;i++)
        {               //collect 1 million samples for SDL processing
			

        pmod1.open("/sys/bus/iio/devices/iio\:device1/in_proximity_raw");


        //pmod1.open("/home/kchai/Desktop/pmodReadingFiles/dummyfile.txt");

                while(!pmod1.eof())
                {               //keep reading contents of file until I reach the end

                        if(pmod1>>number)
                        {
                                 pmod1>>number;
                                 dataFile<< number<<endl;
			 	 cout<< number << endl; //want to see numbers being written to file	 
                        }


                        pmod1.close();

                }

        }

	dataFile.close();

        return 0;

}


