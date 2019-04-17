#ifndef _ASL_WRITER_HPP_
#define _ASL_WRITER_HPP_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#include <fstream>
#include <chrono>
#include <boost/filesystem.hpp>

#define SUBSAMPLE_IMAGES 1

namespace ASL{

    using namespace cv;

    struct CameraCalibration{
    private:
        std::vector<int> dimensions;
        std::vector<float> coeffs;
        std::vector<float> focal;
        std::vector<float> principal;
        Eigen::Matrix4f transform;
    public:
        CameraCalibration(Eigen::Matrix4f transform, rs2_intrinsics intr):
        transform(transform)
        {
            dimensions = {intr.width,intr.height};
            coeffs = {intr.coeffs[0], intr.coeffs[1], intr.coeffs[2], intr.coeffs[3]};
            focal = {intr.fx, intr.fy};
            principal = { intr.ppx, intr.ppy};
        }

        CameraCalibration(){}

        void write(cv::FileStorage& fs) const                        //Write serialization for this class
        {
            fs << "{";
            std::cout << transform << std::endl;
            std::vector<float> t_sc(transform.data(), transform.data() + transform.rows() * transform.cols());
            fs << "T_SC" << t_sc;
            fs << "image_dimension" << dimensions;
            fs << "distortion_type" << "radialtangential";
            fs << "distortion_coefficients" << coeffs;
            fs << "focal_length" << focal;
            fs << "principal_point" << principal;
            fs << "}";
        }

        void read(const cv::FileNode& node)                          //Read serialization for this class
        {
        }


    };

    //These write and read functions must be defined for the serialization in FileStorage to work
    static void write(cv::FileStorage& fs, const std::string&, const ASL::CameraCalibration& x)
    {
        x.write(fs);
    }

    static void read(const cv::FileNode& node, ASL::CameraCalibration& x, const ASL::CameraCalibration& default_value = ASL::CameraCalibration()){
    if(node.empty())
        x = default_value;
    else
        x.read(node);
    }



    ///
    /// This is a modification of the above bridge code
    /// to send data to a file instead of the slam system
    ///
    class Recorder{
    private:
        Recorder(){}
        //output directory to store data
        std::string output_dir_;
        std::string imu_dir_;
        std::string ir1_dir_;
        std::string ir2_dir_;
        //Time syncronization file with imu data and timestamps of images and imu data
        std::ofstream ir1_csv_;
        std::ofstream sync_file_;
        std::ofstream ir2_csv_;
        std::ofstream imu_csv_;
        cv::FileStorage intr_file_;

    public:

        ///
        /// Create the recorder and attempt to open the file
        /// will happily overwrite existing files. 
        ///
        Recorder(std::string output_dir):
        output_dir_(output_dir)
        {
            boost::filesystem::path dir(output_dir_.c_str());
            if(boost::filesystem::create_directory(dir))
            {
                std::cerr<< "Directory Created: "<<output_dir_<<std::endl;
            }
            imu_dir_ = output_dir_+"imu0/";
            boost::filesystem::path imu_dir(imu_dir_.c_str());
            if(boost::filesystem::create_directory(imu_dir))
            {
                std::cerr<< "Directory Created: "<<imu_dir_<<std::endl;
            }

            ir1_dir_ = output_dir_+"cam0/";
            boost::filesystem::path ir1_dir(ir1_dir_.c_str());
            if(boost::filesystem::create_directory(ir1_dir))
            {
                std::cerr<< "Directory Created: "<<ir1_dir_<<std::endl;
            }

            ir2_dir_ = output_dir_+"cam1/";
            boost::filesystem::path ir2_dir(ir2_dir_.c_str());
            if(boost::filesystem::create_directory(ir2_dir))
            {
                std::cerr<< "Directory Created: "<<ir2_dir_<<std::endl;
            }

            std::string ir1_data_dir_ = ir1_dir_+"data/";
            boost::filesystem::path ir1_data_dir(ir1_data_dir_.c_str());
            if(boost::filesystem::create_directory(ir1_data_dir))
            {
                std::cerr<< "Directory Created: "<<ir1_data_dir_<<std::endl;
            }

            std::string ir2_data_dir_ = ir2_dir_+"data/";
            boost::filesystem::path ir2_data_dir(ir2_data_dir_.c_str());
            if(boost::filesystem::create_directory(ir2_data_dir))
            {
                std::cerr<< "Directory Created: "<<ir2_data_dir_<<std::endl;
            }

            imu_csv_.open(imu_dir_+"data.csv");
            if(!imu_csv_.is_open()){
                std::stringstream ss;
                ss << output_dir_ << "is not a valid directory";
                throw std::runtime_error(ss.str().c_str());
            }

            ir1_csv_.open(ir1_dir_+"data.csv");
            if(!ir1_csv_.is_open()){
                std::stringstream ss;
                ss << output_dir_ << "is not a valid directory";
                throw std::runtime_error(ss.str().c_str());
            }

            ir2_csv_.open(ir2_dir_+"data.csv");
            if(!ir2_csv_.is_open()){
                std::stringstream ss;
                ss << output_dir_ << "is not a valid directory";
                throw std::runtime_error(ss.str().c_str());
            }

            intr_file_ = cv::FileStorage(output_dir+std::string("intr.yaml"), cv::FileStorage::WRITE);
        }

        ///
        /// Writes a single imu timestamp and data point to file
        ///
        void write_imu(const ImuPair& imu_data){
            unsigned long long now = imu_data.timestamp+14000000000000000;
            if(now<1000000000){
                return;
            }

            imu_csv_ << std::fixed << now << "," 
                << imu_data.gyro[0] << "," << imu_data.gyro[1] << "," << imu_data.gyro[2] << "," 
                << imu_data.accel[0] << "," << imu_data.accel[1] << "," << imu_data.accel[2] << std::endl;
        }

        ///
        /// Creates a image file from data 
        /// and records timestamp in timesync file
        ///
        void write_ir1_image(double timestamp,const cv::Mat& image ){
            //Write image to file
            std::stringstream filename;
            unsigned long long now = timestamp+14000000000000000;
            if(now>1000000000){
                filename << std::fixed << ir1_dir_ << "data/" << now << ".png";
                imwrite(filename.str(),image);
                //Add timesync data to timesync file
                ir1_csv_ << std::fixed << now << "," << now << ".png" << std::endl;
            }
            
        }

        ///
        /// Creates a image file from data 
        /// and records timestamp in timesync file
        ///
        void write_ir2_image(double timestamp,const cv::Mat& image ){
            //Write image to file
            std::stringstream filename;
            unsigned long long now = timestamp+14000000000000000;
            if(now>1000000000){
                filename << std::fixed << ir2_dir_ << "data/" << now << ".png";
                imwrite(filename.str(),image);
                //Add timesync data to timesync file
                ir2_csv_ << std::fixed << now << "," << now << ".png" << std::endl;
            }
            
        }

        ///
        /// Writes camera information to yaml file
        ///
        void write_camera_intrinsics_and_extrinsics(const std::vector<CameraCalibration>& cameras){
            
            intr_file_ << "cameras" << cameras;
            intr_file_ << "camera_params";
            intr_file_ << "{" << "camera_rate" << 30
                << "sigma_absolute_translation" << 0.0f
                << "sigma_absolute_orientation" << 0.0f
                << "sigma_c_relative_translation" << 0.0f
                << "sigma_c_relative_orientation" << 0.0f
                << "timestamp_tolerance" << 0.005
                << "}";

            
        }

        void write_imu_intrinsics(){
            intr_file_ << "imu_params";
            std::vector<float> acc_bias = {0,0,0};
            std::vector<float> t_bs = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
            intr_file_ << "{" << "a_max" << 176.0
                << "g_max" << 7.8
                << "sigma_g_c" << 0.0 //sqrt(intr.gyro.noise_variances[0])
                << "sigma_a_c" << 0.0 //sqrt(intr.acc.noise_variances[0])
                << "sigma_bg" << 0.0 //sqrt(intr.gyro.bias_variances[0])
                << "sigma_ba" << 0.0 //sqrt(intr.acc.bias_variances[0])
                << "sigma_gw_c" << 4.0e-6
                << "sigma_aw_c" << 4.0e-5
                << "tau" << 3600.0
                << "g" << 9.81007
                << "a0" << acc_bias
                << "imu_rate" << 200
                << "T_BS" << t_bs;

        }

        /// 
        /// Close files
        ///
        void close(){
            if(sync_file_.is_open())
                sync_file_.close();

            intr_file_.release();
        }
        
        ~Recorder(){
            if(sync_file_.is_open())
                sync_file_.close();
        }
    };


    
};

#endif