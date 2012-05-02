package de.meilhaus.medriver;

import java.security.InvalidParameterException;

public class MeIOStreamTrigger{
	private int iAcqStartTrigType;
	private int iAcqStartTrigEdge;
	private int iAcqStartTrigChan;
	private long iAcqStartTicks;
	private int[] iAcqStartArgs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	private int iScanStartTrigType;
	private long iScanStartTicks;
	private int[] iScanStartArgs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	private int iConvStartTrigType;
	private long iConvStartTicks;
	private int[] iConvStartArgs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	private int iScanStopTrigType;
	private int iScanStopCount;
	private int iAcqStopTrigType;
	private int iAcqStopCount;
	private int iFlags;

	public MeIOStreamTrigger(){
		this.iAcqStartTrigType = 0;
		this.iAcqStartTrigEdge = 0;
		this.iAcqStartTrigChan = 0;
		this.iAcqStartTicks = 0;
		this.iScanStartTrigType = 0;
		this.iScanStartTicks = 0;
		this.iConvStartTrigType = 0;
		this.iConvStartTicks = 0;
		this.iScanStopTrigType = 0;
		this.iScanStopCount = 0;
		this.iAcqStopTrigType = 0;
		this.iAcqStopCount = 0;
		this.iFlags = 0;
	}

	public MeIOStreamTrigger(
			int iAcqStartTrigType,
			int iAcqStartTrigEdge,
			int iAcqStartTrigChan,
			long iAcqStartTicks,
			int[] iAcqStartArgs,
			int iScanStartTrigType,
			long iScanStartTicks,
			int[] iScanStartArgs,
			int iConvStartTrigType,
			long iConvStartTicks,
			int[] iConvStartArgs,
			int iScanStopTrigType,
			int iScanStopCount,
			int iAcqStopTrigType,
			int iAcqStopCount,
			int iFlags){
		this.iAcqStartTrigType = iAcqStartTrigType;
		this.iAcqStartTrigEdge = iAcqStartTrigEdge;
		this.iAcqStartTrigChan = iAcqStartTrigChan;
		this.iAcqStartTicks = iAcqStartTicks;
		if(iAcqStartArgs.length != 10){
			throw new InvalidParameterException("iAcqStartArgs length is not 10.");
		}
		this.iAcqStartArgs = iAcqStartArgs;

		this.iScanStartTrigType = iScanStartTrigType;
		this.iScanStartTicks = iScanStartTicks;
		if(iScanStartArgs.length != 10){
			throw new InvalidParameterException("iScanStartArgs length is not 10.");
		}
		this.iScanStartArgs = iScanStartArgs;

		this.iConvStartTrigType = iConvStartTrigType;
		this.iConvStartTicks = iConvStartTicks;
		if(iConvStartArgs.length != 10){
			throw new InvalidParameterException("iConvStartArgs length is not 10.");
		}
		this.iConvStartArgs = iConvStartArgs;

		this.iScanStopTrigType = iScanStopTrigType;
		this.iScanStopCount = iScanStopCount;

		this.iAcqStopTrigType = iAcqStopTrigType;
		this.iAcqStopCount = iAcqStopCount;

		this.iFlags = iFlags;
	}

	public int getAcqStartTrigType(){
		return this.iAcqStartTrigType;
	}
	public int getAcqStartTrigEdge(){
		return this.iAcqStartTrigEdge;
	}
	public int getAcqStartTrigChan(){
		return this.iAcqStartTrigChan;
	}
	public long getAcqStartTicks(){
		return this.iAcqStartTicks;
	}
	public int[] getAcqStartArgs(){
		return this.iAcqStartArgs;
	}
	public int getScanStartTrigType(){
		return this.iScanStartTrigType;
	}
	public long getScanStartTicks(){
		return this.iScanStartTicks;
	}
	public int[] getScanStartArgs(){
		return this.iScanStartArgs;
	}
	public int getConvStartTrigType(){
		return this.iConvStartTrigType;
	}
	public long getConvStartTicks(){
		return this.iConvStartTicks;
	}
	public int[] getConvStartArgs(){
		return this.iConvStartArgs;
	}
	public int getScanStopTrigType(){
		return this.iScanStopTrigType;
	}
	public int getScanStopCount(){
		return this.iScanStopCount;
	}
	public int getAcqStopTrigType(){
		return this.iAcqStopTrigType;
	}
	public int getAcqStopCount(){
		return this.iAcqStopCount;
	}
	public int getFlags(){
		return this.iFlags;
	}

	public void setAcqStartTrigType(int iAcqStartTrigType){
		this.iAcqStartTrigType = iAcqStartTrigType;
	}
	public void setAcqStartTrigEdge(int iAcqStartTrigEdge){
		this.iAcqStartTrigEdge = iAcqStartTrigEdge;
	}
	public void setAcqStartTrigChan(int iAcqStartTrigChan){
		this.iAcqStartTrigChan = iAcqStartTrigChan;
	}
	public void setAcqStartTicks(long iAcqStartTicks){
		this.iAcqStartTicks = iAcqStartTicks;
	}
	public void setAcqStartArgs(int[] iAcqStartArgs){
		if(iAcqStartArgs.length != 10){
			throw new InvalidParameterException("iAcqStartArgs length is not 10.");
		}
		this.iAcqStartArgs = iAcqStartArgs;
	}
	public void setScanStartTrigType(int iScanStartTrigType){
		this.iScanStartTrigType = iScanStartTrigType;
	}
	public void setScanStartTicks(long iScanStartTicks){
		this.iScanStartTicks = iScanStartTicks;
	}
	public void setScanStartArgs(int[] iScanStartArgs){
		if(iScanStartArgs.length != 10){
			throw new InvalidParameterException("iScanStartArgs length is not 10.");
		}
		this.iScanStartArgs = iScanStartArgs;
	}
	public void setConvStartTrigType(int iConvStartTrigType){
		this.iConvStartTrigType = iConvStartTrigType;
	}
	public void setConvStartTicks(long iConvStartTicks){
		this.iConvStartTicks = iConvStartTicks;
	}
	public void setConvStartArgs(int[] iConvStartArgs){
		if(iConvStartArgs.length != 10){
			throw new InvalidParameterException("iConvStartArgs length is not 10.");
		}
		this.iConvStartArgs = iConvStartArgs;
	}
	public void setScanStopTrigType(int iScanStopTrigType){
		this.iScanStopTrigType = iScanStopTrigType;
	}
	public void setScanStopCount(int iScanStopCount){
		this.iScanStopCount = iScanStopCount;
	}
	public void setAcqStopTrigType(int iAcqStopTrigType){
		this.iAcqStopTrigType = iAcqStopTrigType;
	}
	public void setAcqStopCount(int iAcqStopCount){
		this.iAcqStopCount = iAcqStopCount;
	}
	public void setFlags(int iFlags){
		this.iFlags = iFlags;
	}
}
