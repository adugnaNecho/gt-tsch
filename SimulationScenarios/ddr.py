import string
LogFile = open("loglistener.txt")
SentNumber=0
RecvNumber=0
onTimeRecvNumber=0
check=0
TotalE2EDelay=0
DropNumber=0;


for line in LogFile:
            if "SendData" in line:
                                SentNumber=SentNumber+1
                                columns = line.split()
                                test=columns[1]
                                test=test.replace('ID:','')
                                recv="RecvData" + " " + columns[3]
                                LogFile1 = open("loglistener.txt")
                                for line1 in LogFile1:
                                                if recv in line1:  
                                                        columns1 = line1.split()
                                                        SentTime=int(columns[0])
                                                        RecvTime=int(columns1[0])
                                                        E2EDelay=0
                                                        if RecvTime > SentTime: 
                                                                        E2EDelay=RecvTime-SentTime
                                                                        if E2EDelay <10000:
                                                                                        TotalE2EDelay=TotalE2EDelay+E2EDelay
                                                                                        RecvNumber=RecvNumber+1
                                                                                        break
                            
                                LogFile1.close()
		      
LogFile.close()

Loss=SentNumber-RecvNumber
DeliveryRatio=float(RecvNumber)/float( SentNumber)
MeanE2EDelay=float(TotalE2EDelay)/float(RecvNumber)

print("Sent Number =" , SentNumber, "\n")
print ("loss=" , Loss,"\n")
print ("Recv Number =" , RecvNumber,"\n")
print ("Delivery Ratio =" , DeliveryRatio,"\n")
print ("Average E2E Delay = " , MeanE2EDelay,"\n")




