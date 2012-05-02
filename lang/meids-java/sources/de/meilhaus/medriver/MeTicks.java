package de.meilhaus.medriver;

public class MeTicks{
	private long iTicks;
	private double dTime;

	public MeTicks(){
		iTicks = 0;
		dTime = 1.0;
	}

	public MeTicks(long iTicks){
		this.iTicks = iTicks;
		this.dTime = 1.0;
	}

	public long getTicks(){
		return this.iTicks;
	}
	public double getFrequency(){
		return (1 / this.dTime);
	}
	public double getTime(){
		return this.dTime;
	}

	public void setTicks(long iTicks){
		this.iTicks = iTicks;
	}
	public void setFrequency(double dFrequency){
		this.dTime = 1 / dFrequency;
	}
	public void setTime(double dTime){
		this.dTime = dTime;
	}
}
