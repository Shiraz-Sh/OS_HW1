#pragma once

#include <string>

/**
 * Retrieve the IP of a given network interface
 * @param iface the name of the network interface
 */
std::string get_ip(const char* iface);

/**
 * Retrieve the subnet mask of a given network interface
 * @param iface the name of the network interface
 */
std::string get_subnet_mask(const char* iface);

// std::string get_default_gateway();
// std::string get_dns_server();

/**
 * Check if the interface iface exists
 * @param iface_name the name of the network interface
 * @returns true if exists
 */
bool interface_exists(const std::string& iface_name);


/**
 * Gets the default gateway
 * @returns the default gateway, else empty string
 */
std::string get_default_gateway();


/**
 * Search for the DNS servers
 * @returns a list of all DNS servers IP in IPv4 format
 */
std::vector<std::string> get_dns_servers();