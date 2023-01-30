

"""
totalSamples=10
actualDistances=[1,2,3,4,5,6,7,8,9,10] # in inches

delay=4

filePath=

def writeToFIle(filePath,lineStr):
    file=open(filePath)
    file.write(lineStr)
    file.close()



serialDevFile='/dev/ttyACM0'
ser=serial.Serial(serialDevFile,9600,timeout=0)



#while (True):
for i in range(totalSamples):
    linestr=""
    for second in range(delay):     #for each sample, want to collect data for 2 seconds
        line=ser.readline()
        linestr= str(i) + ","+line+","+actualDistances[i]+"/n"
        writeToFIle(filePath,linestr)


"""

import serial
import time


totalSamples=10

serialDevFile="/dev/ttyACM0"
filePath="/home/kchai/Desktop/pmodData.txt"

ser=serial.Serial(serialDevFile,9600,timeout=0)

actualDistances=[1,2,3,4,5,6,7,8,9,10] # in inches


for i in range(totalSamples):


    for dataPoint in range(4): #taking 4 data points for each sample
        file=open(filePath,'a')
        time.sleep(1)
        line=ser.readline()

        tempStr=str(line)
        #rawData=tempStr[2:(len(tempStr)-5)]
        #print(rawData)
        print(tempStr)
        file.write(tempStr + '\n')
        #file.write(rawData + "\n")
        #print(str(i+1) + " "+ rawData + "\n")
        #file.write( str(i+1) + " "+ rawData + " "+ str(actualDistances[i])+ "\n")
        file.close()




'''
while(True):
    file=open(filePath,'a')


    time.sleep(1)
    line=ser.readline()

    tempStr=str(line)
    rawData=tempStr[2:(len(tempStr)-5)]
    print(rawData)
    file.write(rawData + "\n")
    file.close()
'''