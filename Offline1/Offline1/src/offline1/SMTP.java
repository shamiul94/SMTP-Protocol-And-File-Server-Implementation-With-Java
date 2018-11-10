/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package offline1;

/**
 *
 * @author Heisenberg
 */
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Base64;
import java.util.Scanner;

public class SMTP {

    public static void showrecom(String state) throws UnknownHostException {
        //System.out.println("Client Current State : " + state);
        if (state.equalsIgnoreCase("begin")) {
            System.out.println("Localhost : " + InetAddress.getLocalHost().getHostName());
            System.out.println("Recommended Command : \"HELO localhost\"");
        }
        if (state.equalsIgnoreCase("wait")) {
            System.out.println("Recommended Command : \"MAIL FROM: <sender_mail_address>\"");
        }
        if (state.equalsIgnoreCase("enveloped without recipients")) {
            System.out.println("Recommended Command : \"RCPT TO: <recipient_mail_address>\"");
        }
        if (state.equalsIgnoreCase("recipients set")) {
            System.out.println("To send email to multiple recipient, Recommended Command : \"RCPT TO: <recipient_mail_address>\"");
            System.out.println("Othherwise, Recommended Command : \"DATA\"");
        }
        if (state.equalsIgnoreCase("writing mail")) {
            System.out.println("Write email body here. Finish the body with newline then full stop then newline (<CR,LF>.<CR,LF>)");
        }
    }

    public static String getResponseMessage(BufferedReader in, String state, String input) throws IOException {
        String response = "";
        String modstate = state;
        System.out.println("waiting for response from server...");

        Boolean ready = false;
        long t = System.currentTimeMillis();
        long end = t + 20000;
        while (System.currentTimeMillis() < end) {
            if (in.ready()) {
                ready = true;
                break;
            }
        }

        if (ready) {
            response = in.readLine();
        } else {
            System.out.println("Server Timeout.");
            System.out.println("Programme Terminated.");
            System.exit(0);
        }

        //Timer timer - new Timer();
        System.out.println("Server response : " + response);

        if (response.substring(0, 1).equalsIgnoreCase("2") || response.substring(0, 1).equalsIgnoreCase("3")) {
            if (state.equalsIgnoreCase("begin")) {
                modstate = "wait";
            }
            if (state.equalsIgnoreCase("wait")) {
                modstate = "enveloped without recipients";
            }
            if (state.equalsIgnoreCase("enveloped without recipients")) {
                modstate = "recipients set";
            }
            if (state.equalsIgnoreCase("recipients set")) {

                if (input.substring(0, 4).equalsIgnoreCase("data")) {
                    modstate = "writing mail";
                } else if (input.substring(0, 7).equalsIgnoreCase("rcpt to")) {
                    modstate = "recipients set";
                }
            }
            if (state.equalsIgnoreCase("writing mail")) {
                modstate = "wait";
            }
        }
        return modstate;
    }

    public static void main(String[] args) throws UnknownHostException, IOException {

        String state = "closed";
        //String mailServer = "localhost";
        String mailServer = "webmail.buet.ac.bd";

        InetAddress mailHost = InetAddress.getByName(mailServer);
        InetAddress localHost = InetAddress.getLocalHost();
        Socket smtpSocket = new Socket(mailHost, 25);

        BufferedReader in = new BufferedReader(new InputStreamReader(smtpSocket.getInputStream()));
        PrintWriter pr = new PrintWriter(smtpSocket.getOutputStream(), true);

        String initialID = in.readLine();
        System.out.println("Message from Server : " + initialID);
        if (initialID.substring(0, 1).equalsIgnoreCase("2") || initialID.substring(0, 1).equalsIgnoreCase("3")) {
            System.out.println("Connection established with Server.");
            state = "begin";
        }

        Scanner userinput = new Scanner(System.in);

        String input = "";
        while (!input.equalsIgnoreCase("quit")) {
            showrecom(state);
            input = "";
            if (state.equalsIgnoreCase("writing mail")) {
                String datainput = "";
                while (true) {
                    //System.out.println("waiting in while");
                    datainput = userinput.nextLine();

                    if (datainput.equalsIgnoreCase(".")) {
                        break;
                    }

                    input = input + "\n" + datainput;
                }

                //System.out.println(input);

                pr.println("MIME-Version: 1.0");
                pr.println("Content-Type: multipart/mixed; boundary=sep");
                //pr.println();

                pr.println("--sep");

                pr.println("Content-Type: image/jpeg");

                pr.println(input);

                pr.println("--sep");

                //pr.println();
                pr.println("Content-Type: image/jpeg; name=image.jpg");
                pr.println("Content-Disposition: attachment; filename=image.jpg");
                pr.println("Content-Transfer-Encoding: base64");
                //pr.println();

                File file = new File("image.jpg");
                FileInputStream fileInputStreamReader = new FileInputStream(file);
                byte[] bytes = new byte[(int) file.length()];
                fileInputStreamReader.read(bytes);
                byte[] encodedPv = Base64.getEncoder().encode(bytes);
                String encodedfile = new String(encodedPv);
                pr.println(encodedfile);
                pr.println("--sep--");

                pr.println(".");

            } else {
                input = userinput.nextLine();
                pr.println(input);
            }
            if (input.equalsIgnoreCase("rset")) {
                state = "wait";
            } else {
                state = getResponseMessage(in, state, input);
            }
        }
    }

}
