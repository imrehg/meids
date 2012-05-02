package de.meilhaus.medriver;

public class MeRange{
	private int iUnit;
	private double dMin;
	private double dMax;
	private int iMaxData;
	private int iRange;

	public MeRange(){
		iUnit = 0;
		dMin = 0.0;
		dMax = 0.0;
		iMaxData = 0;
		iRange = 0;
	}

	public MeRange(int iUnit, double dMin, double dMax){
		this.iUnit = iUnit;
		this.dMin = dMin;
		this.dMax = dMax;
		this.iMaxData = 0;
		this.iRange = 0;
	}

	public void setUnit(int iUnit){
		this.iUnit = iUnit;
	}
	public void setMin(double dMin){
		this.dMin = dMin;
	}
	public void setMax(double dMax){
		this.dMax = dMax;
	}
	public void setMaxData(int iMaxData){
		this.iMaxData = iMaxData;
	}
	public void setRange(int iRange){
		this.iRange = iRange;
	}

	public int getUnit(){
		return this.iUnit;
	}
	public double getMin(){
		return this.dMin;
	}
	public double getMax(){
		return this.dMax;
	}
	public int getMaxData(){
		return this.iMaxData;
	}
	public int getRange(){
		return this.iRange;
	}
}
