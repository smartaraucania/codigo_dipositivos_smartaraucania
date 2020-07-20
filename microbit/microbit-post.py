import serial
import urllib3
import time

http = urllib3.PoolManager()
# microbit = serial.Serial('/dev/ttyACM0', baudrate=9600, timeout=1.0) #linux/mac
microbit = serial.Serial('COM11', baudrate=115200, timeout=3.0) #windows

while True:
    data = microbit.readline() # read a byte string
    dataDecode = data.decode() # decode byte string into Unicode 
    dataClean = dataDecode.rstrip()  # remove \n and \r
    dataSeparated = dataClean.split(",") #separate the string with a ","

    #Serial data input format
    #Pressure, Temperature, Humidity, Digital Temperature, Sound, UV, light Intensity
    #['97.99', '18.74', '50.47', '20', '2', '0.55', '42']
    
    #The received information from Serial is passed to the format that the IoT-Agent can read
    bodyBME280 = "temperature|"+dataSeparated[1]+"|humidity|"+dataSeparated[2]+"|pressure|"+dataSeparated[0]
    bodyDTemperature = "DTemperature|"+dataSeparated[3]
    bodySound =  "sound|"+dataSeparated[4]
    bodyUV = "UV|"+dataSeparated[5]
    bodyLIntensity = "light|"+dataSeparated[6]

    # print(dataSeparated)

    # POST requests are now made for each particular sensor
    http.request('POST', 'https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i=environment001:BME280&d='+ bodyBME280, 
    headers={'Content-Type': 'text/plain'})
    time.sleep(0.3)

    http.request('POST', 'https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i=temperature001:DS18B20&d='+ bodyDTemperature, 
    headers={'Content-Type': 'text/plain'})
    time.sleep(0.3)

    http.request('POST', 'https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i=sound001&d='+ bodySound, 
    headers={'Content-Type': 'text/plain'})
    time.sleep(0.3)

    http.request('POST', 'https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i=UVsensor001:ML8511&d='+ bodyUV, 
    headers={'Content-Type': 'text/plain'})
    time.sleep(0.3)

    http.request('POST', 'https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i=light001:TCS34725&d='+ bodyLIntensity, 
    headers={'Content-Type': 'text/plain'})
    time.sleep(1.5)
    
