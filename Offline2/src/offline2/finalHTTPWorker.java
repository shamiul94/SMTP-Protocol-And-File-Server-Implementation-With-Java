/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package offline2;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.StringTokenizer;

/**
 *
 * @author Heisenberg
 */
public class finalHTTPWorker implements Runnable {

    private Socket socket;
    InputStream is = null;
    OutputStream os = null;
    int id;

    public finalHTTPWorker(Socket s, int id) {
        this.socket = s;
        this.id = id;

        try {
            is = s.getInputStream();
            os = s.getOutputStream();
        } catch (IOException ex) {
            System.out.println("Couldn't open IO Stream.");
        }

    }

    public String getClientResponse(BufferedReader br) {
        char[] ch;
        ch = new char[1000];

        try {
            br.read(ch);
            return new String(ch);
        } catch (IOException ex) {
            System.err.println("BufferedReader br couldn't read client's response.");
            return null;
        }
    }

    public void generateSubmittedForm(String fileName, String userName) {
        String str;
        str = "<html>\n"
                + "	<head>\n"
                + "		<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
                + "	</head>\n"
                + "	<body>\n"
                + "		<h1> Welcome to CSE 322 Offline 1</h1>\n"
                + "		<h2> HTTP REQUEST TYPE-> " + "POST" + "</h2>\n"
                + "		<h2> Post-> " + userName + "</h2>\n"
                + "		<form name=\"input\" action=\"http://localhost:6789/form_submited\" method=\"post\">\n"
                + "		Your Name: <input type=\"text\" name=\"user\">\n"
                + "		<input type=\"submit\" value=\"Submit\">\n"
                + "		</form>\n"
                + "	</body>\n"
                + "</html>";

        File file = new File(fileName);
        try {
            file.createNewFile();

            try (FileWriter fw = new FileWriter(file)) {
                fw.write(str);
                fw.flush();
            }
        } catch (IOException ex) {
            System.err.println("Couldn't open " + fileName + " file.");

        }
    }

    public boolean sendFile(BufferedReader br, PrintWriter pw, String fileName) {

        File file = new File(fileName);
        if (!file.exists()) {
            pw.println("HTTP/1.0 404 Not found");
            pw.flush();

            pw.println("MIME-Version: 1.0");
            pw.println("Content-Type: text/html");
            pw.println();
            pw.flush();

            System.err.println("404 File Not Found.");

            String notFound;
            notFound = "<html>\n"
                    + "	<head>\n"
                    + "	</head>\n"
                    + "	<body>\n"
                    + "		<h1> 404 NOT FOUND. </h1>\n"
                    + "	</body>\n"
                    + "</html>";
            pw.println(notFound);
            pw.flush();

            System.err.println("File " + fileName + " not found.");

            try {
                is.close();
                os.close();
                socket.close();
            } catch (IOException ex) {
                System.err.println("Couldn't close IO Stream and Socket.");
            }

            return false;
        }

        String fileType = fileName.substring(fileName.indexOf(".") + 1);

        pw.println("HTTP/1.0 200 OK");
        pw.flush();

        System.out.println("File " + fileName + " found.");

        String contentType = "html";

        if (fileType.equals("html") || fileType.equals("css")
                || fileType.equals("txt") || fileType.equals("js")) {

            String suffix;
            switch (fileType) {
                case "txt":
                    suffix = "plain";
                    break;
                case "html":
                    suffix = "html";
                    break;
                case "css":
                    suffix = "css";
                    break;
                case "js":
                    suffix = "javascript";
                    break;
                default:
                    suffix = "html";
                    break;
            }
            contentType = "text/" + suffix;

        } else if (fileType.equals("gif") || fileType.equals("png")
                || fileType.equals("jpeg") || fileType.equals("jpg")
                || fileType.equals("bmp") || fileType.equals("webp")) {

            contentType = "image/" + fileType;
            if (fileType.equals("jpg")) {
                contentType = "image/" + "jpeg";
            }
        } else if (fileType.equals("pdf") || fileType.equals("xml")) {
            contentType = "application/" + fileType;
        }

        int fileLength = (int) file.length();

        pw.println("MIME-Version: 1.0");
        pw.println("Content-Type: " + contentType);
        pw.println("Content-Length: " + fileLength);
        pw.println();
        pw.flush();

        justSendFile(file);

        return true;
    }

    public void justSendFile(File file) {

        try {
            byte[] content;
            FileInputStream fis;
            fis = new FileInputStream(file);
            BufferedInputStream bis = new BufferedInputStream(fis);
            OutputStream sendFileStream = os;

            int fileLength = (int) file.length();

            long current = 0;

            while (current != fileLength) {
                int size = 10000;
                if (fileLength - current >= size) {
                    current += size;
                } else {
                    size = (int) (fileLength - current);
                    current = fileLength;
                }

                content = new byte[size];
                bis.read(content, 0, size);
                sendFileStream.write(content);
            }
            sendFileStream.flush();
            System.out.println("File sent successfully!");

            is.close();
            os.close();
            socket.close();

        } catch (IOException ex) {
            System.err.println("Couldn't send the file.");
        }

    }

    @Override
    public void run() {
        System.out.println("This is id: " + id);
        BufferedReader br = new BufferedReader(new InputStreamReader(is));
        PrintWriter pw = new PrintWriter(os);

        String response;
        response = getClientResponse(br);

        if (response.startsWith("POST")) {
            String fileName, userName, tempResponse;
            int indexOfUser;

            tempResponse = response;

            tempResponse = tempResponse.trim();
            System.out.println(tempResponse);

            indexOfUser = tempResponse.indexOf("user=");
            userName = tempResponse.substring(indexOfUser + 5);
            System.out.println("GOT IT:" + userName);

            StringTokenizer tokenizer = new StringTokenizer(tempResponse, " /"); //takes " " and "/" both as delims.
            tokenizer.nextToken();
            fileName = tokenizer.nextToken();
            fileName = fileName + ".html";

            generateSubmittedForm(fileName, userName);
            sendFile(br, pw, fileName);

        } else if (response.startsWith("GET")) {

            String fileName, tempResponse;
            tempResponse = response;
            StringTokenizer tokenizer = new StringTokenizer(tempResponse, " /"); //takes " " and "/" both as delims.
            tokenizer.nextToken();
            fileName = tokenizer.nextToken();

            if (fileName.equals("HTTP")) {
                fileName = "index.html";
            }

            sendFile(br, pw, fileName);
        }

        /*
      GET /index.hhh HTTP/1.1
Host: localhost:6789
Connection: keep-alive
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,/*;q=0.8
Accept-Encoding: gzip, deflate, br
Accept-Language: en-US,en;q=0.9
         */
    }

}
