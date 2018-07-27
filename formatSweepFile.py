
import numpy as np

def formatSweepFile(inputSweeps, startInt):

    a = open(inputSweeps, 'r')
    b = open('outputSweeps_all.ini','w')
    
    sweeps = a.readlines()
    headerLine = sweeps[0]
    titles = np.array(headerLine.split(","))
    d = {0:"name", 1:"startRA", 2:"endRA", 5:"startDec", 6:"endDec", 7:"date"}
#generate dictionary to pick out the titles you want
#write the lines, using \n to generate a new line whenever necessary
    print(titles)
    index = startInt-1
    for line in sweeps:
	sweepPars = line.split(",")
	#columnInd = np.where(titles == '"Run"')
	for i in d:
    	    param = sweepPars[i]
	    slash = "\ "  #.replace(" ", "")
	    newLine = str(index) + slash.replace(" ", "") + d[i] + "=" + param.replace('"','')
	    if d[i] == "date":
		newLine = newLine + "T20:40:00.0"  
	    if (d[i] == "startRA" or d[i] == "startDec"):
		#if (newLine != "0.00s" or newLine != "*00.0s"):
		if (len(newLine) < 21): 
		    newLine = newLine + "0.00s"
            if (d[i] == "endRA" or d[i] == "endDec"):
		#if (newLine != "0.00s" or newLine != "*00.0s"):
		if (len(newLine) < 18): 
		    newLine = newLine + "0.00s"
            newLine += "\n"
	    print(newLine)
	    b.write(newLine)
	
        #print(sweepPars[columnInd[0]])
	#print(d[1])
	index = index + 1
	#print(len(sweeps[0]))

    b.close()	
    a.close()



formatSweepFile('all_sweep_data.csv', 0)
#0\date=1787-01-24T20:40:00.0

