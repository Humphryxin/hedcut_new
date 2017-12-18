#include "hedcut.h"
#include "simple_svg_1.0.0.hpp"

using namespace std;

//This variable is defined in wcvt.h
int argc_GPU;
char** argv_GPU;

inline string getImageName(const string & img_name)
{
	//
	string output;
	int dot_pos = img_name.rfind(".");
	int slash_pos = img_name.rfind("/");
	if (slash_pos == string::npos)
		output = img_name.substr(0, dot_pos);
	else
		output = img_name.substr(slash_pos + 1, dot_pos - slash_pos - 1);
	//

	return output;
}
int closest_odd_nubmer(int n)
{
	if(n%2 == 0)
		return n-1;
	return n;
}
void fast_compute(string img_filename)
{
	cv::Mat image = cv::imread(img_filename.c_str(), CV_LOAD_IMAGE_COLOR);
	std::cout << "in fast_compute function" << std::endl;
	cv::Mat grayscale;
	cv::cvtColor(image, grayscale, CV_BGR2GRAY);
	//cout << image.rows << "\t " << image.cols <<  endl;
	std::list<HedcutDisk> disks;
	//push_back
	//cout << grayscale.size() << endl;
	//cout << image.cols <<endl;
	

	for(int i=0;i<image.cols-20;i+=20)
	{
		for(int j=0;j<image.rows-20;j+= 20)
		{

			// 
			int trans_x = i, trans_y = j;
			// translate to global coordinate

			float total = 0.0f;
			for(int x=0; x<20; x++){
				for(int y=0; y<20; y++)
				{
					//cout <<  (int) (grayscale.at<uchar>(j+x, i+y)) << " ";
					//cout << (255 - grayscale.at<uchar>(j+x, i+y))*1.0f / 255 << " ";
					total += (255 - (int)grayscale.at<uchar>(j+x, i+y))*1.0f / 255;
				}
			}
			
			//cout << "total: " <<  total << " ";
			int index = (int)(total/5+1);
			int new_index = closest_odd_nubmer(index);
			// load svg : xml read-in for C++
			stringstream ss; 
			string svg_filename;
			ss << "../data/solid-black_" << new_index << ".svg";
			svg_filename = ss.str();
			cout << svg_filename << endl;
			
			ifstream svg_file (svg_filename, ios::in);
			if (! svg_file.is_open())
				{ cout << "Error opening file"; exit (1); }
			char buffer[256];
			svg_file.getline(buffer,100);
			svg_file.getline(buffer,100);
			svg_file.getline(buffer,100);
			while (! svg_file.eof() ) {
				svg_file.getline   (buffer,100);
				string line(buffer);
				if(line.compare("</svg>") == 0)
					break;
				istringstream istr; 
				istr.str(line);
				char a;
				for(int t=0;t<11;t++)
					istr >> a;
				float cx, cy, r;
				istr >> cx;
				for(int t=0;t<5;t++)
					istr >> a;
				istr >> cy;
				for(int t=0;t<4;t++)
					istr >> a;
				istr >> r;
				for(int t=0;t<11;t++)
					istr >> a;
				int red, green, blue;
				istr >> red;
				istr >> a;
				istr >> green;
				istr >> a;
				istr >> blue;
				//cout << line << endl;
				//cout << cx << " " << cy << " " << r << " " << red << "," << green << "," << blue << endl;
				// store in vector
				HedcutDisk disk;
				disk.center.x = cx+trans_x; //x = col
				disk.center.y = cy+trans_y; //y = row
				disk.color = cv::Scalar(red, green, blue, 0.0);
				disk.radius = r;
				disks.push_back(disk);
			}
			
			

			
		}
		cout << endl;
	}
	std::cout << "fast_compute function done!" << std::endl;
	
	// write disks to svgs
	//
	//save output to svg
	//
	stringstream ss;
	string img_name = getImageName(img_filename);
	ss << img_name << "-" << "pre-compute" << ".svg";
	svg::Dimensions dimensions(image.size().width, image.size().height);
	svg::Document doc(ss.str(), svg::Layout(dimensions, svg::Layout::TopLeft));
	for (auto& disk :disks)
	{
		uchar r = disk.color.val[0];
		uchar g = disk.color.val[1];
		uchar b = disk.color.val[2];
		svg::Color color(r, g, b);
		//std::cout << disk.radius << "  ";
		svg::Circle circle(svg::Point(disk.center.x, disk.center.y), disk.radius * 2, svg::Fill(color));
		doc << circle;
	}//end for i

	doc.save();
}

int main(int argc, char ** argv)
{

	//get imput image
	if (argc < 2)
	{
		cout << " Usage: " << argv[0] << " [-n #_of_disks -radius disk_radius -uniform_radius -iteration #_of_CVT_iterations -maxD max_CVF_site_displacement] image_file_name" << endl;
		return -1;
	}

	Hedcut hedcut;
	bool debug = false;                //output debugging information
	int sample_size = 1000;

	string img_filename;
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (string(argv[i]) == "-debug") hedcut.debug=debug = true;
			else if (string(argv[i]) == "-n" && i + 1 < argc) sample_size = atoi(argv[++i]);
			else if (string(argv[i]) == "-uniform_radius") hedcut.uniform_disk_size = true;
			else if (string(argv[i]) == "-radius" && i + 1 < argc) hedcut.disk_size = atof(argv[++i]);
			else if (string(argv[i]) == "-iteration" && i + 1 < argc) hedcut.cvt_iteration_limit = atoi(argv[++i]);
			else if (string(argv[i]) == "-maxD" && i + 1 < argc) hedcut.max_site_displacement = atof(argv[++i]);
			else if (string(argv[i]) == "-black" && i + 1 < argc) hedcut.black_disk = true;
			else if (string(argv[i]) == "-avg" && i + 1 < argc) hedcut.average_termination = true;
			else if (string(argv[i]) == "-gpu" && i + 1 < argc) hedcut.gpu = true;
			else if (string(argv[i]) == "-subpixel" && i + 1 < argc) hedcut.subpixels = atoi(argv[++i]);
			else if (string(argv[i]) == "-fast") {hedcut.fast = true;}
			else if (string(argv[i]) == "-edge") {hedcut.edge = true;}
			else
				cerr << "! Error: Unknown flag " << argv[i] << ".  Ignored." << endl;
		}
		else img_filename = argv[i];
	}

	cv::Mat image = cv::imread(img_filename.c_str(), CV_LOAD_IMAGE_COLOR);   // Read the file

	if (!image.data)                              // Check for invalid input
	{
		cout << "! Error: Could not open or find the image" << std::endl;
		return -1;
	}
	if (hedcut.edge)
	{
		// edge decection using ....? 

		// image;
		// to-do-next
		// combine the two results together by directly adding
		// solutoin 2: add huge amount of weight to edge pixels !!!!!!!!!!

		
		cv::Mat contours;
    	cv::Canny(image,contours,10,220);
    	hedcut.contours = contours;
		/*cv::namedWindow("contours", cv::WINDOW_AUTOSIZE);// Create a window for display.
		imshow("contours", contours); 
		cv::waitKey(-1);*/

	}
	if(hedcut.fast)
	{
		fast_compute(img_filename);
		return 1;
	}
	if (hedcut.gpu)	//If the user uses GPU, setup the opengl
	{
		argc_GPU = argc;
		argv_GPU = argv;
	}
	if (debug)
	{
		if (!hedcut.gpu)
		{
			cv::namedWindow("Input image", cv::WINDOW_AUTOSIZE);// Create a window for display.
			imshow("Input image", image);                       // Show our image inside it.
		}
	}

	//
	//compute hedcut
	//

	// possible improvements
	// 1. disk color: grayscale -> Gaussian average?
	// 2. disk size: voronoi size -> ?
	// 3. sample distribution:  guassian -> ? 
	// 4. precomputing multiple levels  ?
	// 5. d = brightness -> d= f(brightness, gradient)?

	if (hedcut.build(image, sample_size) == false)
		cerr << "! Error: Failed to build hedcut. Sorry." << endl;

	if (debug)
	{
		cout << "- Generated " << hedcut.getDisks().size() << " disks" << endl;
	}

	//
	//save output to svg
	//
	stringstream ss;
	string img_name = getImageName(img_filename);
	ss << img_name << "-" << sample_size << ".svg";
	svg::Dimensions dimensions(image.size().width, image.size().height);
	svg::Document doc(ss.str(), svg::Layout(dimensions, svg::Layout::TopLeft));

	for (auto& disk : hedcut.getDisks())
	{
		uchar r = disk.color.val[0];
		uchar g = disk.color.val[1];
		uchar b = disk.color.val[2];
		svg::Color color(r, g, b);
		//cout << disk.radius << "  ";
		svg::Circle circle(svg::Point(disk.center.x, disk.center.y), disk.radius * 2, svg::Fill(color));
		doc << circle;
	}//end for i

	doc.save();

	if (debug)
	{
		cout << "- Saved " << ss.str() << endl;
	}

	return 0;
}


