/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2018
 Author:  kcipi2015@myfit.edu, pbanis2015@my.fit.edu
 Florida Tech, Computer Science
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation; either the current version of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU Affero General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/* ------------------------------------------------------------------------- */
import java.io.*;
import java.net.*;
import java.security.MessageDigest;
import java.text.*;
import java.util.Date;
import java.util.TimeZone;
import java.util.Base64;
import java.util.Scanner;
import gnu.getopt.Getopt;

public class Client {
    private Socket TCPserver;
    private InputStream in;
    private OutputStream out;
    private DatagramSocket UDPserver;
    private String initialTimestamp;
    public final boolean TCP;               //true will define TCP, false will define UDP
    private MessageDigest md;


    /*
     * Client constructor that will start connection in either TCP or UDP
     * INPUT: host: IP address, port: port, initialTimestamp: costume timestamp
     *        TCP: connecting with tcp or udp
     * OUTPUT: void
     */
    public Client(String host, int port, String initialTimestamp, boolean TCP) throws Exception {
        this.TCP = TCP;
        if (TCP) {
            try { //Start TCP connection
                TCPserver = new Socket(host, port);
                in = TCPserver.getInputStream();
                out = TCPserver.getOutputStream();
            } catch (Exception e) {
                System.out.println(e);
                throw new Exception();
            }
        } else {
            try { //Start UDP connection
                UDPserver = new DatagramSocket();
                UDPserver.connect(InetAddress.getByName(host), port);
            } catch (Exception e) {
                System.out.println(e);
                throw new Exception();
            }
        }
        try { //Send initial message if available
            md = MessageDigest.getInstance("SHA-256");
        } catch (Exception e) {

        }
        this.initialTimestamp = initialTimestamp;
    }
    
    /*
     * messageHandler feeds the message recived by the user to the propriate function
     * INPUT: input: message
     * OUTPUT: void
     */
    private void messageHandler(String input) throws IOException {
        if (input.equals("PEERS?")) {   //handle PEERS?
            PEERS(input);
        } else if (input.indexOf("PEER") == 0) {   //handle PEER
            PEER(input);
        } else {
            GOSSIP(input);  //handle GOSSIP
        }
    }
    
    /*
     * GOSSIP sends GOSSIP command to server
     * INPUT: s: message
     * OUTPUT: void
     */
    private void GOSSIP(String s) throws IOException {
        String hash, timestamp;
        timestamp = generateTimestamp();
        //if an initial timestamp was given, the first message and only the first message will use it
        if (initialTimestamp != null) {
            timestamp = initialTimestamp;
            //wipes condition so we only use an argument timestamp once, otherwise generate them
            initialTimestamp = null;
        }
        hash = generateHash(s, timestamp);
        //               GOSSIP:   [sha]   :     [timestamp] :  [message] %
        String toSend = "GOSSIP:" + hash + ":" + timestamp + ":" + s + "%";
        sendMessage(toSend);
    }
    /*
     * generateTimestamp generates timestamp
     * INPUT: void
     * OUTPUT: timestamp as a String
     */
    private String generateTimestamp() {
        TimeZone.setDefault(TimeZone.getTimeZone("UTC"));
        SimpleDateFormat formatter = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss-SSS");
        String timestamp = formatter.format(new Date()); timestamp +="Z";
        return timestamp;
    }
    /*
     * generateHash generates hash
     * INPUT: message: user's message; timestamp: computer's timestamps
     * OUTPUT: hash as a String
     */
    private String generateHash(String message, String timestamp) {
        String temp = timestamp + ":" + message;
        return new String(Base64.getEncoder().encode(md.digest(temp.getBytes())));
    }
    /*
     * sendMessage sends messages feed to the function over TCP or UDp
     * INPUT: s: message
     * OUTPUT: void
     */
    private void sendMessage(String s) throws IOException {
        if (TCP) {
            out.write(s.getBytes());
            out.flush();
        } else {
            DatagramPacket packet = new DatagramPacket(s.getBytes(), s.getBytes().length);
            UDPserver.send(packet);
        }
    }
    /*
     *The user types exactly what is sent online, except for the hash
     *Thus a PEER string is provided by the user, we do not prompt for input
     * Further, we can let the server handle error checking for us, so we don't
     * need to verify s for a valid form here
     */
    private void PEER(String s) throws IOException {
        sendMessage(s);
    }
    /*
     * PEERS handles PEERS command
     * INPUT: s: PEERS command
     * OUTPUT: void
     */
    private void PEERS(String s) throws IOException {
        sendMessage(s);
        receiveResponse();   
    }
    /*
     * receiveResponse listens to the server to get the response over TCP or UDP
     * INPUT: void
     * OUTPUT: void
     */
    private void receiveResponse() throws IOException {
       //networking code
        byte[] temp = new byte[1024];
        if (TCP) {
            while (in.read(temp) != -1) {
            //read
            }
            
        } else {
            DatagramPacket t = new DatagramPacket(temp, temp.length);
            UDPserver.receive(t);
            temp = t.getData();
        }
       //call displayResponse(responseFromServer)
       displayResponse(new String(temp));
    }

    /*
     * displayResponse prints the server response to the standard output
     * INPUT: response: response recived from the server
     * OUTPUT: void
     */
    private void displayResponse(String response) {
        System.out.println(response.trim());
    }

    public static void main(String[] args) {
        //Parssing client arguments
        Getopt g = new Getopt("Client", args, "p:s:m:t:TU");
        int c, port = -1;
        boolean TCP = false;
        String serverIP = "", message = null, timestamp = null; //GOSSIP expects null for proper functionality
        while ((c = g.getopt()) != -1) {
            switch(c) {
                case 'p':
                    port = Integer.parseInt(g.getOptarg());
                    break;
                case 's':
                    serverIP = g.getOptarg();
                    break;
                case 'm':
                    message = g.getOptarg();
                    break;
                case 't':
                    timestamp = g.getOptarg();
                    break;
                case 'T':
                    TCP = true;
                    break;
                case 'U':
                    TCP = false;
                    break;
                default:
                    System.out.print("Error on getopt\n");
            }
        }
        //User has to provide IP and PORT
        if (serverIP == "" || port == -1) {
            System.out.println("ERROR: Please provide IP and PORT");
            return;
        }
        
        try {
            Client client = new Client(serverIP, port, timestamp, TCP);
            //initial message
            try {
                if (message != null) {
                    client.messageHandler(message);
                }
            } catch (Exception e) {}
            //all user input
            Scanner stdin = new Scanner(System.in);
            while (stdin.hasNext()) {
                try {
                    client.messageHandler(stdin.nextLine().trim());
                } catch (Exception e) {
                    stdin.close();//prevent resource leak
                }
            }
            stdin.close(); //prevent resource leak


        } catch (Exception e) {
            System.err.println("Failure to establish connection");
        }

    }

}
