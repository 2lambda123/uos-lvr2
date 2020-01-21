#include "lvr2/io/ScanIOUtils.hpp"
#include <boost/regex.hpp>
#include <opencv2/opencv.hpp>
#include <set>

namespace lvr2
{

std::set<size_t> loadPositionIdsFromDirectory(
    const boost::filesystem::path& path
)
{
    std::set<size_t> positions;
    boost::filesystem::path scan_path = path / "scans";
    const boost::regex scan_dir_filter("^([0-9]{5}).*$");

    boost::filesystem::directory_iterator end_itr;
    for(boost::filesystem::directory_iterator it(scan_path); it != end_itr; ++it )
    {
        if( !boost::filesystem::is_directory( it->path() ) ) continue;
        boost::smatch what;
        if( !boost::regex_match( it->path().filename().string(), what, scan_dir_filter ) ) continue;
        positions.insert(static_cast<size_t>(std::stoul(what[1])));
    }

    return positions;
}

std::set<size_t> loadCamIdsFromDirectory(
    const boost::filesystem::path& path,
    const size_t& positionNr
)
{
    std::set<size_t> cam_ids;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << positionNr;
    boost::filesystem::path scan_images_dir = path / "scan_images" / ss.str();

    if(boost::filesystem::exists(scan_images_dir))
    {
        const boost::regex image_filter("^(\\d+)_(\\d+)\\.(bmp|dib|jpeg|jpg|jpe|jp2|png|pbm|pgm|ppm|sr|ras|tiff|tif)$");

        boost::filesystem::directory_iterator end_itr;
        for(boost::filesystem::directory_iterator it(scan_images_dir); it != end_itr; ++it )
        {
            if( !boost::filesystem::is_regular_file( it->path() ) ) continue;

            boost::smatch what;
            if( !boost::regex_match( it->path().filename().string(), what, image_filter ) ) continue;

            cam_ids.insert(static_cast<size_t>(std::stoul(what[1])));

        }
    }

    return cam_ids;
}

std::set<size_t> loadImageIdsFromDirectory(
    const boost::filesystem::path& path,
    const size_t& positionNr,
    const size_t& camNr
)
{
    std::set<size_t> img_ids;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << positionNr;
    boost::filesystem::path scan_images_dir = path / "scan_images" / ss.str();

    
    if(boost::filesystem::exists(scan_images_dir))
    {
        const boost::regex image_filter("^(\\d+)_(\\d+)\\.(bmp|dib|jpeg|jpg|jpe|jp2|png|pbm|pgm|ppm|sr|ras|tiff|tif)$");

        boost::filesystem::directory_iterator end_itr;
        for(boost::filesystem::directory_iterator it(scan_images_dir); it != end_itr; ++it )
        {
            if( !boost::filesystem::is_regular_file( it->path() ) ) continue;

            boost::smatch what;
            if( !boost::regex_match( it->path().filename().string(), what, image_filter ) ) continue;

            if(static_cast<size_t>(std::stoul(what[1])) == camNr)
            {
                img_ids.insert(static_cast<size_t>(std::stoul(what[2])));
            }
        }
    }


    return img_ids;
}

void writeScanMetaYAML(const boost::filesystem::path& path, const Scan& scan)
{
    
    YAML::Node meta;
    meta = scan;

    std::ofstream out(path.c_str());
    if (out.good())
    {
        out << meta;
    }
    else
    {
        std::cout << timestamp << "Warning: Unable to open " 
                  << path.string() << "for writing." << std::endl;
    }
    
}

void saveScanToDirectory(
    const boost::filesystem::path& path, 
    const Scan& scan, 
    const size_t& positionNr)
{
    // Create full path from root and scan number
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << positionNr;
    boost::filesystem::path directory = path / "scans" / ss.str();
    
    // Check if directory exists, if not create it and write meta data
    // and into the new directory

    if(!boost::filesystem::exists(path))
    {
        std::cout << timestamp << "Creating " << path << std::endl;
        boost::filesystem::create_directory(path);
    }

    if(!boost::filesystem::exists(path / "scans")) {
        std::cout << timestamp << "Creating " << path / "scans" << std::endl;
        boost::filesystem::create_directory(path / "scans");
    }

    if(!boost::filesystem::exists(directory)) 
    {
        std::cout << timestamp << "Creating " << directory << std::endl;
        boost::filesystem::create_directory(directory);
    } 

    // Create yaml file with meta information
    std::cout << timestamp << "Creating scan meta.yaml..." << std::endl;
    writeScanMetaYAML(directory / "meta.yaml", scan);

    // Save points
    std::string scanFileName( (directory / "scan.ply").string() );
    std::cout << timestamp << "Writing " << scanFileName << std::endl;
    
    ModelPtr model(new Model());
    if(scan.m_points)
    {
        model->m_pointCloud = scan.m_points;
        PLYIO io;
        io.save(model, scanFileName);
    }
    else
    {
        std::cout << timestamp << "Warning: Point cloud pointer is empty!" << std::endl;
    }
   
    
}

bool loadScanFromDirectory(
    const boost::filesystem::path& path, 
    Scan& scan, const size_t& positionNr, bool load)
{
    if(boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << positionNr;
        boost::filesystem::path position_directory = path / "scans" / ss.str();
        
        if(boost::filesystem::exists(position_directory))
        {
            // Load meta data
            boost::filesystem::path meta_yaml_path = position_directory / "meta.yaml";
            std::cout << timestamp << "Reading " << meta_yaml_path << std::endl;
            loadScanMetaInfoFromYAML(meta_yaml_path, scan);


            // Load scan data
            boost::filesystem::path scan_path = position_directory / "scan.ply";
            std::cout << timestamp << "Reading " << scan_path << std::endl;

            if (boost::filesystem::exists(scan_path))
            {
                scan.m_scanFile = scan_path;
                scan.m_scanRoot = path; // TODO: Check root dir or scan dir??
                if (load)
                {
                    PLYIO io;
                    ModelPtr model = io.read(scan_path.string());
                    scan.m_points = model->m_pointCloud;
                    scan.m_pointsLoaded = true;
                }
                else
                {
                    scan.m_pointsLoaded = false;
                }
                return true;
            }
            else
            {
                std::cout << "Warning: scan.ply not found in directory "
                          << scan_path << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << timestamp
                      << "Warning: Scan directory " << position_directory << " "
                      << "does not exist." << std::endl;
            return false;
        }
    }
    else
    {
        std::cout << timestamp 
                  << "Warning: '" << path.string() 
                  << "' does not exist or is not a directory." << std::endl; 
        return false;
    }
}

void loadScanMetaInfoFromYAML(const boost::filesystem::path& path, Scan& scan)
{
    YAML::Node meta = YAML::LoadFile(path.string());
    scan = meta.as<Scan>();
}


void saveScanToHDF5(const std::string filename, const size_t& positionNr)
{

}

bool loadScanFromHDF5(const std::string filename, const size_t& positionNr)
{
    return true;
}

void saveScanImageToDirectory(
    const boost::filesystem::path& path, 
    const ScanImage& image,
    const size_t& positionNr,
    const size_t& camNr,
    const size_t& imageNr)
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << positionNr;
    boost::filesystem::path scanimage_directory = path / "scan_images" / ss.str();

    if(boost::filesystem::exists(path))
    {
        if(!boost::filesystem::exists(path / "scan_images"))
        {
            std::cout << timestamp << "Creating " << path / "scan_images" << std::endl;
            boost::filesystem::create_directory(path / "scan_images"); 
        }

        // Create scan image directory if necessary
        if(!boost::filesystem::exists(scanimage_directory))
        {
            std::cout << timestamp << "Creating " << scanimage_directory << std::endl;
            boost::filesystem::create_directory(scanimage_directory);        
        }

        // Create in image folder for current position if necessary
        
        // Save image in .png format
        std::stringstream image_str;
        image_str << camNr << "_" << imageNr << ".png";
        boost::filesystem::path image_path = scanimage_directory / image_str.str();
        
        std::cout << timestamp << "Saving " << image_path << std::endl;
        cv::imwrite(image_path.string(), image.image);

        // Save calibration yaml
        std::stringstream meta_str;
        meta_str << camNr << "_" << imageNr << ".yaml";
        boost::filesystem::path meta_path = scanimage_directory / meta_str.str();
        std::cout << timestamp << "Saving " << meta_path << std::endl;
        writePinholeModelToYAML(meta_path, image.camera);
    }
    else
    {
        std::cout << timestamp
                  << "Warning: Scan directory for scan image does not exist: "
                  << path << std::endl;
    }
    
}

void writePinholeModelToYAML(
    const boost::filesystem::path& path, const PinholeCameraModeld& model)
{
    YAML::Node meta;
    meta = model;
    
    std::ofstream out(path.c_str());
    if (out.good())
    {
        out << meta;
    }
    else
    {
        std::cout << timestamp << "Warning: Unable to open " 
                  << path.string() << "for writing." << std::endl;
    }

}

void loadPinholeModelFromYAML(const boost::filesystem::path& path, PinholeCameraModeld& model)
{
    YAML::Node model_file = YAML::LoadFile(path.string());
    model = model_file.as<PinholeCameraModeld>();
}

bool loadScanImageFromDirectory(
    const boost::filesystem::path& path, 
    ScanImage& image, 
    const size_t& positionNr, 
    const size_t& camNr,
    const size_t& imageNr)
{
    // Convert position and image number to strings
    stringstream pos_str;
    pos_str << std::setfill('0') << std::setw(5) << positionNr;

    // Construct a path to image directory and check
    boost::filesystem::path scan_image_dir = path / "scan_images" / pos_str.str();
    if(boost::filesystem::exists(scan_image_dir))
    {
        std::stringstream yaml_file, image_file;
        yaml_file << camNr << "_" << imageNr << ".yaml";
        image_file << camNr << "_" << imageNr << ".png";

        boost::filesystem::path meta_path = scan_image_dir / yaml_file.str();
        boost::filesystem::path image_path = scan_image_dir / image_file.str();

        if(!boost::filesystem::exists(meta_path))
        {
            std::cout << timestamp << "Could not load meta file of scan/cam/img: " << positionNr << "/" << camNr << "/" << imageNr << std::endl;
            return false;
        }

        std::cout << timestamp << "Loading " << image_path << std::endl;
        image.image = cv::imread(image_path.string(), 1);
        image.image_file = image_path;

        std::cout << timestamp << "Loading " << meta_path << std::endl;
        loadPinholeModelFromYAML(meta_path, image.camera);
    }
    else
    {
        std::cout << timestamp << "Warning: Image directory does not exist: "
                  << scan_image_dir << std::endl;
        return false;
    }

    return true;
}

void saveScanPositionToDirectory(const boost::filesystem::path& path, const ScanPosition& position, const size_t& positionNr)
{  
    // Save scan data
    std::cout << timestamp << "Saving scan postion " << positionNr << std::endl;
    if(position.scan)
    {
        saveScanToDirectory(path, *position.scan, positionNr);
    }
    else
    {
        std::cout << timestamp << "Warning: Scan position " << positionNr
                  << " contains no scan data." << std::endl;
    }
    
    // Save rgb camera recordings
    for(size_t cam_id = 0; cam_id < position.cams.size(); cam_id++)
    {
        // store each image of camera
        for(size_t img_id = 0; img_id < position.cams[cam_id]->images.size(); img_id++ )
        {
            saveScanImageToDirectory(path, *position.cams[cam_id]->images[img_id], positionNr, cam_id, img_id);
        }
    }
}

void get_all(
    const boost::filesystem::path& root, 
    const string& ext, vector<boost::filesystem::path>& ret)
{
    if(!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root))
    {
        return;
    } 

    boost::filesystem::directory_iterator it(root);
    boost::filesystem::directory_iterator endit;

    while(it != endit)
    {
        if(boost::filesystem::is_regular_file(*it) && it->path().extension() == ext)
        {
            ret.push_back(it->path().filename());
        } 
        ++it;
    }
}

bool loadScanPositionFromDirectory(
    const boost::filesystem::path& path,
    ScanPosition& position, 
    const size_t& positionNr)
{
    bool scan_read = false;
    bool images_read = false;

    std::cout << timestamp << "Loading scan position " << positionNr << std::endl;
    Scan scan;
    if(!loadScanFromDirectory(path, scan, positionNr, true))
    {
        std::cout << timestamp << "Warning: Scan position " << positionNr 
                  << " does not contain scan data." << std::endl;
    } else {
        position.scan = scan;
    }


    boost::filesystem::path img_directory = path / "scan_images";
    if(boost::filesystem::exists(img_directory))
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << positionNr;
        boost::filesystem::path scanimage_directory = img_directory / ss.str();
        if(boost::filesystem::exists(scanimage_directory))
        {
            std::set<size_t> cam_ids = loadCamIdsFromDirectory(path, positionNr);

            for(const size_t& cam_id : cam_ids)
            {
                ScanCameraPtr cam(new ScanCamera);
                std::set<size_t> img_ids = loadImageIdsFromDirectory(path, positionNr, cam_id);
                for(const size_t& img_id : img_ids)
                {
                    ScanImagePtr img(new ScanImage);
                    loadScanImageFromDirectory(path, *img, positionNr, cam_id, img_id);
                    cam->images.push_back(img);
                }
                position.cams.push_back(cam);
            }
        } else {
            std::cout << timestamp << "Warning: Specified scan has no images." << std::endl;
        }

    }
    else
    {
        std::cout << timestamp << "Scan position " << positionNr 
                  << " has no images." << std::endl;
    }
    return true;
}

void saveScanProjectToDirectory(const boost::filesystem::path& path, const ScanProject& project)
{
    boost::filesystem::create_directory(path);

    YAML::Node yaml;
    yaml["pose"] = project.pose;
    std::ofstream out( (path / "meta.yaml").string() );
    if (out.good())
    {
        out << yaml;
    }
    else
    {
        std::cout << timestamp << "Warning: Unable to open " 
                  << path.string() << "for writing." << std::endl;
    }

    for(size_t i = 0; i < project.positions.size(); i++)
    {
        saveScanPositionToDirectory(path, *project.positions[i], i);
    }
}

bool loadScanProjectFromDirectory(const boost::filesystem::path& path, ScanProject& project)
{
    YAML::Node meta = YAML::LoadFile((path / "meta.yaml").string() );    
    project.pose = meta["pose"].as<Transformd>();


    boost::filesystem::directory_iterator it(path / "scans");
    boost::filesystem::directory_iterator endit;

    std::set<size_t> scan_pos_ids = loadPositionIdsFromDirectory(path);

    for(size_t scan_pos_id : scan_pos_ids)
    {
        ScanPositionPtr scan_pos(new ScanPosition);
        loadScanPositionFromDirectory(path, *scan_pos, scan_pos_id);
        project.positions.push_back(scan_pos);
    }
    

    return true;
}

} // namespace lvr2