'''
Realizado por Miguel Pérez
Este script permite el traspaso de un archivo de texto (en este caso un html) a C string para poder servirlo a través del servidor web por arduino.
'''

result = 'const char *cstring = '

with open("./Examples/mqtt_remoto/index.html","r") as f:
    for line in f.readlines():
        result = result + "\"" +  line.replace("\n", "").replace('\"', '\\"') + "\"\n" 
        #Agregamos los escapes de comillas y saltos para asegurarnos que quede como un String multilinea para C.

result = result + ";"

w = open("./Examples/mqtt_remoto/Cstring.h", "w")
w.write(result)
w.close()
