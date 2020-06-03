/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//
// Simple example of OLSR routing over some point-to-point links
//
// Network topology
//
// n0 ---- n1 ---- n2 ---- n3 ---- n4
//
// - all links are point-to-point links with indicated one-way BW/delay
// - CBR/UDP flows from n0 to n4, and from n3 to n1
// - UDP packet size of 210 bytes, with per-packet interval 0.00375 sec.
//   (i.e., DataRate of 448,000 bps)
// - DropTail queues
// - Tracing of queues and packet receptions to file "simple-point-to-point-olsr.tr"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"

#define DEBUG   0

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("SimplePointToPointOlsrExample");

int
main (int argc, char *argv[])
{

  double start_t = 1.0;
  double sim_t = 20.0; 


  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
#if DEBUG
  LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);
#endif

  // Set up some default values for the simulation.  Use the

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (210));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("50000kb/s"));

  //DefaultValue::Bind ("DropTailQueue::m_maxPackets", 3000000);

  // Allow the user to override any of the defaults and the above
  // DefaultValue::Bind ()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Here, we will explicitly create four nodes.  In more sophisticated
  // topologies, we could configure a node factory.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (5);
  NodeContainer n01 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer n12 = NodeContainer (c.Get (1), c.Get (2));
  NodeContainer n23 = NodeContainer (c.Get (2), c.Get (3));
  NodeContainer n34 = NodeContainer (c.Get (3), c.Get (4));

  // Enable OLSR
  NS_LOG_INFO ("Enabling OLSR Routing.");
  OlsrHelper olsr;

  //Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  //list.Add (staticRouting, 0);
  list.Add (olsr, start_t);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (c);

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("50000kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("3ms"));
  NetDeviceContainer nd01 = p2p.Install (n01);
  //p2p.SetDeviceAttribute ("DataRate", StringValue ("1500kbps"));
  NetDeviceContainer nd12 = p2p.Install (n12);
  NetDeviceContainer nd23 = p2p.Install (n23);
  NetDeviceContainer nd34 = p2p.Install (n34);

  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.01.0", "255.255.255.0");
  Ipv4InterfaceContainer i01 = ipv4.Assign (nd01);

  ipv4.SetBase ("10.1.12.0", "255.255.255.0");
  Ipv4InterfaceContainer i12 = ipv4.Assign (nd12);

  ipv4.SetBase ("10.1.23.0", "255.255.255.0");
  Ipv4InterfaceContainer i23 = ipv4.Assign (nd23);

  ipv4.SetBase ("10.1.34.0", "255.255.255.0");
  Ipv4InterfaceContainer i34 = ipv4.Assign (nd34);

  ////////////////////////////////////////////////////////////////////////////
  // Create the OnOff applications to send UDP datagrams to the base station
  ////////////////////////////////////////////////////////////////////////////
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)

  // 210 bytes at a rate of 100 Kb/s from n4 to n0
  OnOffHelper onoff1 ("ns3::UdpSocketFactory",
                     InetSocketAddress (i01.GetAddress (0), port));
  onoff1.SetConstantRate (DataRate ("10000kb/s"));
  ApplicationContainer onOffApp1 = onoff1.Install (c.Get (4));
  onOffApp1.Start (Seconds (start_t));
  onOffApp1.Stop (Seconds (sim_t+start_t));

  // 210 bytes at a rate of 100 Kb/s from n3 to n0
  OnOffHelper onoff2 ("ns3::UdpSocketFactory",
                     InetSocketAddress (i01.GetAddress (0), port));
  onoff2.SetConstantRate (DataRate ("10kb/s"));
  ApplicationContainer onOffApp2 = onoff2.Install (c.Get (3));
  onOffApp2.Start (Seconds (start_t));
  onOffApp2.Stop (Seconds (sim_t+start_t));

  // 210 bytes at a rate of 100 Kb/s from n2 to n0
  OnOffHelper onoff3 ("ns3::UdpSocketFactory",
                     InetSocketAddress (i01.GetAddress (0), port));
  onoff3.SetConstantRate (DataRate ("10kb/s"));
  ApplicationContainer onOffApp3 = onoff3.Install (c.Get (2));
  onOffApp3.Start (Seconds (start_t));
  onOffApp3.Stop (Seconds (sim_t+start_t));

  // 210 bytes at a rate of 100 Kb/s from n1 to n0
  OnOffHelper onoff4 ("ns3::UdpSocketFactory",
                     InetSocketAddress (i01.GetAddress (0), port));
  onoff4.SetConstantRate (DataRate ("10kb/s"));
  ApplicationContainer onOffApp4 = onoff4.Install (c.Get (1));
  onOffApp4.Start (Seconds (start_t));
  onOffApp4.Stop (Seconds (sim_t+start_t));

  // Create packet sinks to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  NodeContainer sinks = NodeContainer (c.Get (0));
  ApplicationContainer sinkApps = sink.Install (sinks);
  sinkApps.Start (Seconds (start_t));
  sinkApps.Stop (Seconds (sim_t+start_t));

  // trace results in wireshark
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("simple-point-to-point-olsr.tr"));
  p2p.EnablePcapAll ("simple-point-to-point-olsr");

  Simulator::Stop (Seconds (start_t+sim_t));

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
