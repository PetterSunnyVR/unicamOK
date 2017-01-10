import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;

import org.opencv.core.Core;
import org.opencv.core.Mat;

public class main {	
	static{
		System.loadLibrary(Core.NATIVE_LIBRARY_NAME);
		System.load("D:\\workspace\\JNIMat1\\src\\CVBTest.dll");
	}
	
	public native void FindFeatures(long matAddrGr);
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		Mat mGray = new Mat(); 
		main app = new main();
		app.FindFeatures(mGray.getNativeObjAddr());
		System.out.println(mGray.size());
		System.out.println("Result: "+mGray.size()+" "+app.writeImage(mGray));
		
	}



public boolean writeImage(Mat rgba){
	
	boolean check = false;
	
	byte[] return_buff = new byte[(int) (rgba.total() * rgba.channels())];
	rgba.get(0, 0, return_buff);
	int type;
	
	if(rgba.channels() == 1)
        type = BufferedImage.TYPE_BYTE_GRAY;
    else
        type = BufferedImage.TYPE_3BYTE_BGR;
	
	rgba.get(0,0,return_buff);
	
	
	
	BufferedImage image = new BufferedImage(rgba.width(),rgba.height(), type);
	
	
	image.getRaster().setDataElements(0, 0, rgba.width(), rgba.height(), return_buff);
	
	File outputfile = new File("image1.jpg");
	try {
		ImageIO.write(image, "jpg", outputfile);
		check = true;
	} catch (IOException e) {
		// TODO Auto-generated catch block
		e.printStackTrace();
		System.out.println("Problem while saving image");
	}
	/*// Here we convert into *supported* format
	BufferedImage imageCopy =
	    new BufferedImage(image.getWidth(), image.getHeight(), BufferedImage.TYPE_3BYTE_BGR);
	imageCopy.getGraphics().drawImage(image, 0, 0, null);

	byte[] data = ((DataBufferByte) imageCopy.getRaster().getDataBuffer()).getData();  
	Mat img = new Mat(image.getHeight(),image.getWidth(), CvType.CV_8UC3);
	img.put(0, 0, data);           
	Imgcodecs.imwrite("C:\\File\\input.jpg", img);*/
	return check;
}

}