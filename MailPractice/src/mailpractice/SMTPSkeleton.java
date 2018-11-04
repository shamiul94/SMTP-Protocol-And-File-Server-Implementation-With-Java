/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package mailpractice;

/**
 *
 * @author Heisenberg
 */
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Scanner;

public class SMTPSkeleton {

    public static void getResponseMessage(BufferedReader in, String command) throws IOException {
        String response;
        response = in.readLine();
        System.out.println(command + ": " + response);
    }

    public static void main(String[] args) throws UnknownHostException, IOException {
        String mailServer = "localhost";

        InetAddress mailHost = InetAddress.getByName(mailServer);
        InetAddress localHost = InetAddress.getLocalHost();

        Socket smtpSocket = new Socket(mailHost, 25);
        
        
        

        BufferedReader in = new BufferedReader(new InputStreamReader(smtpSocket.getInputStream()));
        PrintWriter pr = new PrintWriter(smtpSocket.getOutputStream(), true);

        String initialID = in.readLine();
        System.out.println(initialID);

        pr.println("HELO " + localHost.getHostName());
        getResponseMessage(in, "HELO");

        System.out.println("How many mails do you want to send?");

        Scanner intSc = new Scanner(System.in);
        Scanner sc = new Scanner(System.in);
        int t = 0, T = intSc.nextInt();

        String Subject, msgBody;
        
        System.out.println("Subject: ");
        Subject = sc.nextLine();

        System.out.println("Body: ");
        msgBody = sc.nextLine();

        while (t < T) {
            t++;

            String senderMailAddress, receiverMailAddress;

            

            System.out.println("Sender No."+ t +" Mail Address: ");
            senderMailAddress = sc.nextLine();
            
            pr.println("MAIL FROM:<" + senderMailAddress + ">");
            getResponseMessage(in, "MAIL");

            

            System.out.println("Receiver No."+ t +" Mail Address: ");
            receiverMailAddress = sc.nextLine();
            
            pr.println("RCPT TO:<" + receiverMailAddress + ">");
            getResponseMessage(in, "RCPT");
            
            
            

            pr.println("DATA");
            getResponseMessage(in, "DATA");

            pr.println("Subject: " + Subject + "\nFrom: "+ senderMailAddress +"\nTo: "+ receiverMailAddress
                    +"\n\n"+ msgBody +"\n");

            pr.println(".");
            getResponseMessage(in, "Last Dot");

        }
        pr.println("QUIT");
        getResponseMessage(in, "QUIT");
    }
}
