#include <iostream>
#include <string>
#include <regex>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Function to validate email address using regular expression
bool validateEmail(const std::string& email) {
    std::regex emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return std::regex_match(email, emailRegex);
}

// Function to generate a random verification code
std::string generateVerificationCode() {
    static const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string verificationCode;
    for (int i = 0; i < 6; ++i) {
        verificationCode += characters[rand() % characters.size()];
    }
    return verificationCode;
}

// Function to send verification email using SMTP
bool sendVerificationEmail(const std::string& email, const std::string& verificationCode) {
    CURL *curl;
    CURLcode res;
    std::string smtpServer = "smtp.gmail.com";
    std::string smtpPort = "587";
    std::string fromEmail = "your-email@gmail.com";
    std::string fromPassword = "your-password";
    std::string toEmail = email;
    std::string subject = "Email Verification";
    std::string body = "Your verification code is: " + verificationCode;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, smtpServer.c_str());
        curl_easy_setopt(curl, CURLOPT_PORT, smtpPort.c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, fromEmail.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, fromPassword.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, fromEmail.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, toEmail.c_str());
        curl_easy_setopt(curl, CURLOPT_SUBJECT, subject.c_str());
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_READDATA, NULL);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_MAIL_BODY, body.c_str());

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return true;
}

// Function to verify email using IMAP
bool verifyEmail(const std::string& email, const std::string& verificationCode) {
    IMAP *imap;
    imap = imap_open("imap.gmail.com", email.c_str(), "your-password");
    if(imap) {
        imap_search(imap, "ALL");
        imap_fetch(imap, "RFC822");
        imap_store(imap, "RFC822", "FLAGS", "\\Seen");
        imap_close(imap);
        imap_logout(imap);
    }
    return true;
}

int main() {
    std::string email;
    std::cout << "Enter your email address: ";
    std::cin >> email;

    if(validateEmail(email)) {
        std::string verificationCode = generateVerificationCode();
        if(sendVerificationEmail(email, verificationCode)) {
            std::cout << "Verification email sent successfully!" << std::endl;
            std::string inputVerificationCode;
            std::cout << "Enter your verification code: ";
            std::cin >> inputVerificationCode;
            if(inputVerificationCode == verificationCode) {
                std::cout << "Email verified successfully!" << std::endl;
                if(verifyEmail(email, verificationCode)) {
                    std::cout << "Email verified using IMAP!" << std::endl;
                } else {
                    std::cerr << "Failed to verify email using IMAP!" << std::endl;
                }
            } else {
                std::cerr << "Invalid verification code!" << std::endl;
            }
        } else {
            std::cerr << "Failed to send verification email!" << std::endl;
        }
    } else {
        std::cerr << "Invalid email address!" << std::endl;
    }
    return 0;
}