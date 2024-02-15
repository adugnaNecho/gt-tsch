import string
LogFile = open("cooja.testlog")
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
                                LogFile1 = open("cooja.testlog")
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


LogFile2 = open("cooja.testlog")
QueueLoss=0
nodeNumbers1=0
nodeNumbers2=0
nodeNumbers3=0
nodeNumbers4=0
DutyCycle=0
icmpPackets=0
parentChange=0

for line in LogFile2:
 if "drops" in line:
  nodeNumbers1=nodeNumbers1+1
  columns = line.split()
  QueueLoss=QueueLoss + int(columns[3])
 
 if "dutycycle" in line:
  nodeNumbers2=nodeNumbers2+1
  columns = line.split()
  DutyCycle=DutyCycle + int(columns[4])
 
 
 if "icmpPackets" in line:
  nodeNumbers3=nodeNumbers3+1
  columns = line.split()
  icmpPackets=icmpPackets + int(columns[3])

 if "parentChange" in line:
  nodeNumbers4=nodeNumbers4+1
  columns = line.split()
  parentChange=parentChange + int(columns[3])
 
 

LogFile2.close()
     

aveQueueLoss=float(QueueLoss)/float( nodeNumbers1)


print("Average Queue Loss = " , aveQueueLoss)



aveDutyCycle=float(DutyCycle)/float( nodeNumbers2)


print("Average Duty Cycle = " , aveDutyCycle)


aveicmpPackets=float(icmpPackets)/float( nodeNumbers3)


print ("Average Icmp Packets = " , aveicmpPackets)



#aveparentChange=float(parentChange)/float( nodeNumbers4)


#print "Average Parent Changes = " , aveparentChange
avg_recv=float(RecvNumber)/float( 10)
print ("Average Recv= " , avg_recv)

avg_loss=float(Loss)/float( 10)
print ("Average Loss= " , avg_loss)


print ("Average E2E Delay = " , MeanE2EDelay,"\n")

avg_icmp=float(aveicmpPackets)/float( 10)



print ("Avg_icmp = " , avg_icmp,"\n")


print ("Delivery Ratio =" , DeliveryRatio,"\n")

print("Average Duty Cycle = " , aveDutyCycle)
