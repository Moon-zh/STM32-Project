#include <string.h>
#include "includes.h"  
#include "network.h"
RemoteReadStruct ReRead;
RemoteSetStruct ReSet;

void SetRemoteRead(u8 addr,s16 value)
{
	ReRead.Remote_Read[addr] = value;
}
void SetRemotePara(u8 addr,s16 value)
{
	ReSet.Remote_Para[addr] = value;
}
void SetMulRemoteRead(u8 addr,s16* Buf,u8 Num)
{
	memcpy((u8*)&ReSet.Remote_Para[addr],Buf,Num);
}
//只读参数
const u8 Partition1StateStr[16]="1PartitionState\0";
const u8 Partition2StateStr[16]="2PartitionState\0";
const u8 Partition3StateStr[16]="3PartitionState\0";
const u8 Partition4StateStr[16]="4PartitionState\0";
const u8 Partition5StateStr[16]="5PartitionState\0";
const u8 Partition6StateStr[16]="6PartitionState\0";
const u8 Partition7StateStr[16]="7PartitionState\0";
const u8 Partition8StateStr[16]="8PartitionState\0";
const u8 Partition9StateStr[16]="9PartitionState\0";
const u8 Partition10StateStr[17]="10PartitionState\0";
const u8 Partition11StateStr[17]="11PartitionState\0";
const u8 Partition12StateStr[17]="12PartitionState\0";
const u8 Partition13StateStr[17]="13PartitionState\0";
const u8 Partition14StateStr[17]="14PartitionState\0";
const u8 Partition15StateStr[17]="15PartitionState\0";
const u8 Partition16StateStr[17]="16PartitionState\0";
const u8 DeviceStateStr[12]="DeviceState\0";
const u8 IrrCountDownStr[13]="IrrCountDown\0";
const u8 IrrNumStr[7]="IrrNum\0";
const u8 BucketAPercentStr[15]="ABucketPercent\0";
const u8 BucketBPercentStr[15]="BBucketPercent\0";
const u8 BucketCPercentStr[15]="CBucketPercent\0";
const u8 BucketDPercentStr[15]="DBucketPercent\0";
const u8 partition1areaStr[15]="1partitionarea\0";
const u8 partition2areaStr[15]="2partitionarea\0";
const u8 partition3areaStr[15]="3partitionarea\0";
const u8 partition4areaStr[15]="4partitionarea\0";
const u8 partition5areaStr[15]="5partitionarea\0";
const u8 partition6areaStr[15]="6partitionarea\0";
const u8 partition7areaStr[15]="7partitionarea\0";
const u8 partition8areaStr[15]="8partitionarea\0";
const u8 partition9areaStr[15]="9partitionarea\0";
const u8 partition10areaStr[16]="10partitionarea\0";
const u8 partition11areaStr[16]="11partitionarea\0";
const u8 partition12areaStr[16]="12partitionarea\0";
const u8 partition13areaStr[16]="13partitionarea\0";
const u8 partition14areaStr[16]="14partitionarea\0";
const u8 partition15areaStr[16]="15partitionarea\0";
const u8 partition16areaStr[16]="16partitionarea\0";
const u8 CFIPPreFlushTimeStr[17]="CFIPPreFlushTime\0";
const u8 CFIPFertilizeTimeStr[22]="CFIPFertilizationTime\0";
const u8 CFIPPostFlushTimeStr[18]="CFIPPostFlushTime\0";
const u8 CFIPAFertilizationPercentStr[26]="CFIPAFertilizationPercent\0";
const u8 CFIPBFertilizationPercentStr[26]="CFIPBFertilizationPercent\0";
const u8 CFIPCFertilizationPercentStr[26]="CFIPCFertilizationPercent\0";
const u8 CFIPDFertilizationPercentStr[26]="CFIPDFertilizationPercent\0";
const u8 CFIPFertilizationPerAreStr[24]="CFIPFertilizationPerAre\0";
const u8 S1IPPreFlushTimeStr[17]="1SIPPreFlushTime\0";
const u8 S1IPFertilizeTimeStr[22]="1SIPFertilizationTime\0";
const u8 S1IPPostFlushTimeStr[18]="1SIPPostFlushTime\0";
const u8 S1IPAFertilizationPercentStr[26]="1SIPAFertilizationPercent\0";
const u8 S1IPBFertilizationPercentStr[26]="1SIPBFertilizationPercent\0";
const u8 S1IPCFertilizationPercentStr[26]="1SIPCFertilizationPercent\0";
const u8 S1IPDFertilizationPercentStr[26]="1SIPDFertilizationPercent\0";
const u8 S1IPFertilizationPerAreStr[24]="1SIPFertilizationPerAre\0";
const u8 S2IPPreFlushTimeStr[17]="2SIPPreFlushTime\0";
const u8 S2IPFertilizeTimeStr[22]="2SIPFertilizationTime\0";
const u8 S2IPPostFlushTimeStr[18]="2SIPPostFlushTime\0";
const u8 S2IPAFertilizationPercentStr[26]="2SIPAFertilizationPercent\0";
const u8 S2IPBFertilizationPercentStr[26]="2SIPBFertilizationPercent\0";
const u8 S2IPCFertilizationPercentStr[26]="2SIPCFertilizationPercent\0";
const u8 S2IPDFertilizationPercentStr[26]="2SIPDFertilizationPercent\0";
const u8 S2IPFertilizationPerAreStr[24]="2SIPFertilizationPerAre\0";
const u8 S3IPPreFlushTimeStr[17]="3SIPPreFlushTime\0";
const u8 S3IPFertilizeTimeStr[22]="3SIPFertilizationTime\0";
const u8 S3IPPostFlushTimeStr[18]="3SIPPostFlushTime\0";
const u8 S3IPAFertilizationPercentStr[26]="3SIPAFertilizationPercent\0";
const u8 S3IPBFertilizationPercentStr[26]="3SIPBFertilizationPercent\0";
const u8 S3IPCFertilizationPercentStr[26]="3SIPCFertilizationPercent\0";
const u8 S3IPDFertilizationPercentStr[26]="3SIPDFertilizationPercent\0";
const u8 S3IPFertilizationPerAreStr[24]="3SIPFertilizationPerAre\0";
const u8 S4IPPreFlushTimeStr[17]="4SIPPreFlushTime\0";
const u8 S4IPFertilizeTimeStr[22]="4SIPFertilizationTime\0";
const u8 S4IPPostFlushTimeStr[18]="4SIPPostFlushTime\0";
const u8 S4IPAFertilizationPercentStr[26]="4SIPAFertilizationPercent\0";
const u8 S4IPBFertilizationPercentStr[26]="4SIPBFertilizationPercent\0";
const u8 S4IPCFertilizationPercentStr[26]="4SIPCFertilizationPercent\0";
const u8 S4IPDFertilizationPercentStr[26]="4SIPDFertilizationPercent\0";
const u8 S4IPFertilizationPerAreStr[24]="4SIPFertilizationPerAre\0";
u8 CCIDstr[21];
//读写参数
const u8 Partition1EnableStr[17]="1PartitionEnable\0";
const u8 Partition2EnableStr[17]="2PartitionEnable\0";
const u8 Partition3EnableStr[17]="3PartitionEnable\0";
const u8 Partition4EnableStr[17]="4PartitionEnable\0";
const u8 Partition5EnableStr[17]="5PartitionEnable\0";
const u8 Partition6EnableStr[17]="6PartitionEnable\0";
const u8 Partition7EnableStr[17]="7PartitionEnable\0";
const u8 Partition8EnableStr[17]="8PartitionEnable\0";
const u8 Partition9EnableStr[17]="9PartitionEnable\0";
const u8 Partition10EnableStr[18]="10PartitionEnable\0";
const u8 Partition11EnableStr[18]="11PartitionEnable\0";
const u8 Partition12EnableStr[18]="12PartitionEnable\0";
const u8 Partition13EnableStr[18]="13PartitionEnable\0";
const u8 Partition14EnableStr[18]="14PartitionEnable\0";
const u8 Partition15EnableStr[18]="15PartitionEnable\0";
const u8 Partition16EnableStr[18]="16PartitionEnable\0";
const u8 IrrigationTypeStr[15]="IrrigationType\0";
const u8 Partition1SelStr[14]="1PartitionSel\0";
const u8 Partition2SelStr[14]="2PartitionSel\0";
const u8 Partition3SelStr[14]="3PartitionSel\0";
const u8 Partition4SelStr[14]="4PartitionSel\0";
const u8 Partition5SelStr[14]="5PartitionSel\0";
const u8 Partition6SelStr[14]="6PartitionSel\0";
const u8 Partition7SelStr[14]="7PartitionSel\0";
const u8 Partition8SelStr[14]="8PartitionSel\0";
const u8 Partition9SelStr[14]="9PartitionSel\0";
const u8 Partition10SelStr[15]="10PartitionSel\0";
const u8 Partition11SelStr[15]="11PartitionSel\0";
const u8 Partition12SelStr[15]="12PartitionSel\0";
const u8 Partition13SelStr[15]="13PartitionSel\0";
const u8 Partition14SelStr[15]="14PartitionSel\0";
const u8 Partition15SelStr[15]="15PartitionSel\0";
const u8 Partition16SelStr[15]="16PartitionSel\0";
const u8 IrrTimeStr[8]="IrrTime\0";
const u8 IrrProjectStr[11]="IrrProject\0";
const u8 IrrSwitchStr[10]="IrrSwitch\0";
const u8* ReadText[] = 
{
	Partition1StateStr,
	Partition2StateStr,
	Partition3StateStr,
	Partition4StateStr,
	Partition5StateStr,
	Partition6StateStr,
	Partition7StateStr,
	Partition8StateStr,
	Partition9StateStr,
	Partition10StateStr,
	Partition11StateStr,
	Partition12StateStr,
	Partition13StateStr,
	Partition14StateStr,
	Partition15StateStr,
	Partition16StateStr,
	DeviceStateStr,
	IrrCountDownStr,
	IrrNumStr,
	BucketAPercentStr,
	BucketBPercentStr,
	BucketCPercentStr,
	BucketDPercentStr,
	partition1areaStr,
	partition2areaStr,
	partition3areaStr,
	partition4areaStr,
	partition5areaStr,
	partition6areaStr,
	partition7areaStr,
	partition8areaStr,
	partition9areaStr,
	partition10areaStr,
	partition11areaStr,
	partition12areaStr,
	partition13areaStr,
	partition14areaStr,
	partition15areaStr,
	partition16areaStr,
	CFIPPreFlushTimeStr,
	CFIPFertilizeTimeStr,
	CFIPPostFlushTimeStr,
	CFIPAFertilizationPercentStr,
	CFIPBFertilizationPercentStr,
	CFIPCFertilizationPercentStr,
	CFIPDFertilizationPercentStr,
	CFIPFertilizationPerAreStr,
	S1IPPreFlushTimeStr,
	S1IPFertilizeTimeStr,
	S1IPPostFlushTimeStr,
	S1IPAFertilizationPercentStr,
	S1IPBFertilizationPercentStr,
	S1IPCFertilizationPercentStr,
	S1IPDFertilizationPercentStr,
	S1IPFertilizationPerAreStr,
	S2IPPreFlushTimeStr,
	S2IPFertilizeTimeStr,
	S2IPPostFlushTimeStr,
	S2IPAFertilizationPercentStr,
	S2IPBFertilizationPercentStr,
	S2IPCFertilizationPercentStr,
	S2IPDFertilizationPercentStr,
	S2IPFertilizationPerAreStr,
	S3IPPreFlushTimeStr,
	S3IPFertilizeTimeStr,
	S3IPPostFlushTimeStr,
	S3IPAFertilizationPercentStr,
	S3IPBFertilizationPercentStr,
	S3IPCFertilizationPercentStr,
	S3IPDFertilizationPercentStr,
	S3IPFertilizationPerAreStr,
	S4IPPreFlushTimeStr,
	S4IPFertilizeTimeStr,
	S4IPPostFlushTimeStr,
	S4IPAFertilizationPercentStr,
	S4IPBFertilizationPercentStr,
	S4IPCFertilizationPercentStr,
	S4IPDFertilizationPercentStr,
	S4IPFertilizationPerAreStr
};
const u8* SetText[] = 
{
	Partition1EnableStr,
	Partition2EnableStr,
	Partition3EnableStr,
	Partition4EnableStr,
	Partition5EnableStr,
	Partition6EnableStr,
	Partition7EnableStr,
	Partition8EnableStr,
	Partition9EnableStr,
	Partition10EnableStr,
	Partition11EnableStr,
	Partition12EnableStr,
	Partition13EnableStr,
	Partition14EnableStr,
	Partition15EnableStr,
	Partition16EnableStr,
	IrrigationTypeStr,
	Partition1SelStr,
	Partition2SelStr,
	Partition3SelStr,
	Partition4SelStr,
	Partition5SelStr,
	Partition6SelStr,
	Partition7SelStr,
	Partition8SelStr,
	Partition9SelStr,
	Partition10SelStr,
	Partition11SelStr,
	Partition12SelStr,
	Partition13SelStr,
	Partition14SelStr,
	Partition15SelStr,
	Partition16SelStr,
	IrrTimeStr,
	IrrProjectStr,
	IrrSwitchStr
};

