import java.io.*;
import java.net.*;
import java.security.MessageDigest;
import java.text.*;
import java.util.Date;
import java.util.TimeZone;
import java.util.Base64;
import gnu.getopt.Getopt;
//import org.apache.commons.codec.digest.DigestUtils;

public class Client {
    private Socket TCPserver;
    private InputStream in;
    private OutputStream out;
    private DatagramSocket UDPserver;
    private String initialTimestamp;
    public final boolean TCP; //true will define TCP, false will define UDP
    private MessageDigest md;


    //We distinguish the TCP and UDP constructor by having the TCP constructor require a useless int
    public Client(String host, int port, String initialTimestamp, String initialMessage, boolean TCP) throws Exception {
        this.TCP = TCP;
        if (TCP) {
            try {
                TCPserver = new Socket(host, port);
                in = TCPserver.getInputStream();
                out = TCPserver.getOutputStream();
            } catch (Exception e) {
                System.out.println(e);
                throw new Exception();
            }
        } else {
            try {
                UDPserver = new DatagramSocket();
                UDPserver.connect(InetAddress.getByName(host), port);
            } catch (Exception e) {
                System.out.println(e);
                throw new Exception();
            }
        }
        try {
            md = MessageDigest.getInstance("SHA-256");
        } catch (Exception e) {

        }
        this.initialTimestamp = initialTimestamp;
        if (initialMessage != null) {
            messageHandler(initialMessage);
        }      
    }
    
    //use to test invidual functions without needing connections
    public Client() {
        TCP = false;
        try {
            md = MessageDigest.getInstance("SHA-256");
        } catch (Exception e) {

        }
 
    }

   
    private void messageHandler(String input) throws IOException {
        if (input.equals("PEERS?")) {
            PEERS(input);
        } else if (input.indexOf("PEER") == 0) {
            PEER(input);
        } else {
            GOSSIP(input);
        }
    }

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

    private String generateTimestamp() {
        TimeZone.setDefault(TimeZone.getTimeZone("UTC"));
        SimpleDateFormat formatter = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss-SSS");
        String timestamp = formatter.format(new Date()); timestamp +="Z";
        return timestamp;
    }

    private String generateHash(String message, String timestamp) {
        String temp = timestamp + ":" + message;
        return new String(Base64.getEncoder().encode(md.digest(temp.getBytes())));
    }

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
     *I'd prefer the user types exactly what is sent online, except for the hash
     *Thus a PEER string is provided by the user, we do not prompt for input
     * Further, we can let the server handle error checking for us, so we don't
     * need to verify s for a valid form here
     */
    private void PEER(String s) throws IOException {
        sendMessage(s);
    }

    private void PEERS(String s) throws IOException {
        sendMessage(s);
        receiveResponse();   
    }
    
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


    private void displayResponse(String response) {
        System.out.println(response);
    }

    public static void main(String[] args) {
<<<<<<< HEAD
        //process args
=======
        //Parssing client arguments
        Getopt g = new Getopt("GossipServer", args, "p:s:m:t:TU");
        int c, port = -1;
        boolean TCP = false;
        String serverIP = "", message = "", timestamp = "";
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
        System.out.println(TCP);

>>>>>>> d615e7fc2a742fb5c44fc4ee3b0e721c6ca456c9

        //initialize connection

        //while (!EOF) loop 
        //    read input
        //    messageHandler(input)
        //end loop

    }

}