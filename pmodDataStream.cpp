
#include <iostream>
#include <fstream>
#include <string>


using namespace std; 

int main()
{
       
       

        ifstream pmod1;
	string number;

	//pmod1.open("/sys/bus/iio/devices/iio\:device1/in_proximity_raw");
	

	while(true)
	{		//streamdata from file continuously

	pmod1.open("/sys/bus/iio/devices/iio\:device1/in_proximity_raw");
	
	
	//pmod1.open("/home/kchai/Desktop/pmodReadingFiles/dummyfile.txt");

		while(!pmod1.eof())
		{		//keep reading contents of file until I reach the end
		
			if(pmod1>>number)
			{
				 pmod1>>number;
			   	 cout<< number<<endl;
			}
			

			pmod1.close();

		}

	}

        return 0;

}

