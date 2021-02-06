import numpy as np
import re
import csv



def dec_format(deg,m):
	degnum=int(deg)
	degnum=90-degnum
	minnum=int(m)
	if minnum!=0:
		
		if degnum>0:
			degnum-=1
			minnum=60-minnum
		else:
			minnum=-minnum
	if minnum<0 or degnum<0:
		negative_str='-'
	else:
		negative_str=''
	degstr=negative_str+tp(str(abs(degnum)))+"d"+tp(str(abs(minnum)))+"m0.00s"
	return(degstr)

def tp(s):
	if len(s)==1:
		s='0'+s
	return s

def date_format(datestr):
	m,d,y=datestr.split('-')
	d=tp(d)
	m=tp(m)
	return('-'.join([y,m,d]))

def formatSweepFile(inputSweeps, startInt):
	a = open(inputSweeps, 'r')
	b = open('default_sweep.ini','w')
	
	outputstr=''
	
	index=startInt
	with open(inputSweeps) as csvfile:
		reader=csv.DictReader(csvfile)
		for row in reader:
			date=date_format(row['date'])+"T20:40:00.0"
			startRA=tp(row['startRAh'])+'h'+tp(row['startRAm'])+'m'+'0.00s'
			endRA=tp(row['endRAh'])+'h'+tp(row['endRAm'])+'m'+'0.00s'
			startDec=dec_format(row['startDecd'],row['startDecm'])
			endDec=dec_format(row['endDecd'],row['endDecm'])
			name='Sweep'+row['name']
			block_dict={'name':name,'startRA':startRA,'endRA':endRA,'startDec':startDec,'endDec':endDec,'date':date}
			for k in block_dict:
				outputstr+='\n'+str(index)+'\\'+k+'='+block_dict[k]
			index+=1
	headers='[General]\narrow_scale=1.5\nsweep_count=%d\nsweeps_version=2.0\nuse_decimal_degrees=false\nuse_semi_transparency=false\n\n[sweep]' %(index+1)
	b.write(headers+outputstr)
	a.close()
	b.close()

formatSweepFile('sweep_data.csv', 0)