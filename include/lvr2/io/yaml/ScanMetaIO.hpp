
#ifndef LVR2_IO_YAML_SCANMETA_IO_HPP
#define LVR2_IO_YAML_SCANMETA_IO_HPP

#include <yaml-cpp/yaml.h>
#include "lvr2/types/ScanTypes.hpp"
#include "MatrixIO.hpp"

namespace YAML {  

/**
 * YAML-CPPs convert specialization
 * 
 * example: 
 */

// WRITE SCAN PARTIALLY
template <>
struct convert<lvr2::Scan> 
{

    /**
     * Encode Eigen matrix to yaml. 
     */
    static Node encode(const lvr2::Scan& scan) {
        Node node;
        
        node["type"] = "scan";

        node["start_time"]  = scan.m_startTime;
        node["end_time"] = scan.m_endTime;

        node["pose_estimate"] = scan.m_poseEstimation;
        node["registration"] = scan.m_registration;

        Node config;
        config["theta"] = Load("[]");
        config["theta"].push_back(scan.m_thetaMin);
        config["theta"].push_back(scan.m_thetaMax);

        config["phi"] = Load("[]");
        config["phi"].push_back(scan.m_phiMin);
        config["phi"].push_back(scan.m_phiMax);

        config["v_res"] = scan.m_vResolution;
        config["h_res"] = scan.m_hResolution;

        config["num_points"] = scan.m_numPoints;
        node["config"] = config;

        return node;
    }

    static bool decode(const Node& node, lvr2::Scan& scan) {
        
        if(node["type"].as<std::string>() != "scan")
        {
            return false;
        }

        scan.m_startTime = node["start_time"].as<float>();
        scan.m_endTime = node["end_time"].as<float>();
        scan.m_poseEstimation = node["pose_estimate"].as<lvr2::Transformd>();
        scan.m_registration = node["registration"].as<lvr2::Transformd>();
        
        const Node& config = node["config"];

        
        scan.m_thetaMin = config["theta"][0].as<float>();
        scan.m_thetaMax = config["theta"][1].as<float>();

        scan.m_phiMin = config["phi"][0].as<float>();
        scan.m_phiMax = config["phi"][1].as<float>();


        scan.m_vResolution = config["v_res"].as<float>();
        scan.m_hResolution = config["h_res"].as<float>();

        scan.m_numPoints = config["num_points"].as<float>();

        return true;
    }

};

}  // namespace YAML

#endif // LVR2_IO_YAML_SCANMETA_IO_HPP

