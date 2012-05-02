package examples;

import de.meilhaus.medriver.*;

public class Query {
	public static void main(String[] args){
		MeDriver drv = new MeDriver();

		try{
			drv.meOpen(0);

			int nd = drv.meQueryNumberDevices();
			int ns = 0;
			int nc = 0;
			int nr = 0;
			System.out.println("Number of devices = " + nd);

			for(int i = 0; i < nd; i++){
				ns = drv.meQueryNumberSubdevices(i);
				System.out.println("Device " + i + " Number of subdevices = " + ns);

				for(int j = 0; j < ns; j++){
					nc = drv.meQueryNumberChannels(i, j);
					System.out.println("Device " + i + " Subdevice " + j + " Number of channels = " + nc);

					MeType type = new MeType();
					drv.meQuerySubdeviceType(i, j, type);

					if((type.getType() == MeDriver.ME_TYPE_AO) || (type.getType() == MeDriver.ME_TYPE_AI)){
						nr = drv.meQueryNumberRanges(i, j, 0, MeDriver.ME_UNIT_ANY);
						System.out.println("Device " + i + " Subdevice " + j + " Number of ranges = " + nr);
						MeRange range = new MeRange();

						for(int k = 0; k < nr; k++){
							range.setRange(k);
							drv.meQueryRangeInfo(i, j, 0, range);
							System.out.println("Device " + i + " Subdevice " + j + " Range " + k + "  Unit = " + range.getUnit());
							System.out.println("Device " + i + " Subdevice " + j + " Range " + k + "  Min = " + range.getMin());
							System.out.println("Device " + i + " Subdevice " + j + " Range " + k + "  Max = " + range.getMax());
							System.out.println("Device " + i + " Subdevice " + j + " Range " + k + "  Max Data = " + range.getMaxData());
						}
					}
				}
			}

			drv.meClose(0);
		}
		catch(java.io.IOException e){
			e.printStackTrace(System.err);
		}
	}
}
