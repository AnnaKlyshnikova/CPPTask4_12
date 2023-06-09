#include <afxwin.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

class Logger {
private:
    std::ofstream logFile;

public:
    Logger(const std::string& fileName) {
        logFile.open(fileName, std::ios::app);
    }

    ~Logger() {
        logFile.close();
    }

    void log(const std::string& message, int level = 0) {
        std::time_t now = std::time(nullptr);
        char timestamp[100];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        logFile << "[" << timestamp << "] ";

        switch (level) {
        case 1:
            logFile << "[INFO] ";
            break;
        case 2:
            logFile << "[WARNING] ";
            break;
        case 3:
            logFile << "[ERROR] ";
            break;
        default:
            break;
        }

        logFile << message << std::endl;
    }
};

class Contact {
private:
    std::string name;
    std::vector<std::string> phoneNumbers;
    std::string group;

public:
    Contact(const std::string& contactName, const std::string& contactGroup)
        : name(contactName), group(contactGroup) {}

    void addPhoneNumber(const std::string& phoneNumber) {
        phoneNumbers.push_back(phoneNumber);
    }

    std::string getName() const {
        return name;
    }

    std::string getGroup() const {
        return group;
    }

    void display() const {
        std::cout << "Name: " << name << ", Group: " << group << std::endl;
        std::cout << "Phone Numbers: ";
        for (const auto& phoneNumber : phoneNumbers) {
            std::cout << phoneNumber << " ";
        }
        std::cout << std::endl;
    }
};

class Reminder {
private:
    std::string title;
    std::string explanation;
    std::string time;
    std::vector<Contact*> relatedContacts;

public:
    Reminder(const std::string& reminderTitle, const std::string& reminderTime)
        : title(reminderTitle), time(reminderTime) {}

    void setExplanation(const std::string& reminderExplanation) {
        explanation = reminderExplanation;
    }

    void addRelatedContact(Contact* contact) {
        relatedContacts.push_back(contact);
    }

    void display() const {
        std::cout << "Title: " << title << std::endl;
        std::cout << "Time: " << time << std::endl;
        std::cout << "Explanation: " << explanation << std::endl;
        std::cout << "Related Contacts: ";
        for (const auto& contact : relatedContacts) {
            std::cout << contact->getName() << " ";
        }
        std::cout << std::endl;
    }
};

class CNoteApp : public CWinApp {
public:
    virtual BOOL InitInstance();
};

class CMainFrame : public CFrameWnd {
private:
    std::vector<Contact*> contacts;
    std::vector<Reminder*> reminders;
    Logger logger;

public:
    CMainFrame() : logger("log.txt") {
        Create(NULL, _T("Записная книжка"), WS_OVERLAPPEDWINDOW, CRect(0, 0, 800, 600));
    }

    afx_msg void OnClose() {
        SaveData();
        CFrameWnd::OnClose();
    }

    afx_msg void OnAddContact() {
        CString name;
        CString group;
        CString phoneNumber;

        CDialog dlg;
        dlg.Create(IDD_ADD_CONTACT_DIALOG);

        if (dlg.DoModal() == IDOK) {
            name = dlg.GetDlgItemText(IDC_NAME_EDIT);
            group = dlg.GetDlgItemText(IDC_GROUP_EDIT);
            phoneNumber = dlg.GetDlgItemText(IDC_PHONE_NUMBER_EDIT);

            Contact* contact = new Contact((LPCTSTR)name, (LPCTSTR)group);
            contact->addPhoneNumber((LPCTSTR)phoneNumber);
            contacts.push_back(contact);

            CString logMessage;
            logMessage.Format(_T("Contact added: %s"), name);
            logger.log((LPCTSTR)logMessage, 1);
        }
    }

    afx_msg void OnAddReminder() {
        CString title;
        CString time;
        CString explanation;

        CDialog dlg;
        dlg.Create(IDD_ADD_REMINDER_DIALOG);

        if (dlg.DoModal() == IDOK) {
            title = dlg.GetDlgItemText(IDC_TITLE_EDIT);
            time = dlg.GetDlgItemText(IDC_TIME_EDIT);
            explanation = dlg.GetDlgItemText(IDC_EXPLANATION_EDIT);

            Reminder* reminder = new Reminder((LPCTSTR)title, (LPCTSTR)time);
            reminder->setExplanation((LPCTSTR)explanation);
            reminders.push_back(reminder);

            CString logMessage;
            logMessage.Format(_T("Reminder added: %s"), title);
            logger.log((LPCTSTR)logMessage, 1);
        }
    }

    afx_msg void OnViewContacts() {
        for (const auto& contact : contacts) {
            contact->display();
        }
    }

    afx_msg void OnViewReminders() {
        for (const auto& reminder : reminders) {
            reminder->display();
        }
    }

    DECLARE_MESSAGE_MAP()

private:
    void LoadData() {
        std::ifstream inputFile("data.txt");
        if (inputFile.is_open()) {
            std::string line;
            std::string currentSection;

            while (std::getline(inputFile, line)) {
                if (line == "[Contacts]") {
                    currentSection = "Contacts";
                }
                else if (line == "[Reminders]") {
                    currentSection = "Reminders";
                }
                else {
                    if (currentSection == "Contacts") {
                        std::string name = line;
                        std::string group;
                        std::getline(inputFile, group);

                        Contact* contact = new Contact(name, group);

                        std::string phoneNumber;
                        while (std::getline(inputFile, phoneNumber) && !phoneNumber.empty()) {
                            contact->addPhoneNumber(phoneNumber);
                        }

                        contacts.push_back(contact);
                    }
                    else if (currentSection == "Reminders") {
                        std::string title = line;
                        std::string time;
                        std::getline(inputFile, time);

                        Reminder* reminder = new Reminder(title, time);

                        std::string explanation;
                        std::getline(inputFile, explanation);

                        reminder->setExplanation(explanation);

                        std::string relatedContacts;
                        std::getline(inputFile, relatedContacts);

                        for (const auto& contact : contacts) {
                            if (relatedContacts.find(contact->getName()) != std::string::npos) {
                                reminder->addRelatedContact(contact);
                            }
                        }

                        reminders.push_back(reminder);
                    }
                }
            }

            inputFile.close();
        }
    }

    void SaveData() {
        std::ofstream outputFile("data.txt");
        if (outputFile.is_open()) {
            outputFile << "[Contacts]" << std::endl;
            for (const auto& contact : contacts) {
                outputFile << contact->getName() << std::endl;
                outputFile << contact->getGroup() << std::endl;

                for (const auto& phoneNumber : contact->getPhoneNumbers()) {
                    outputFile << phoneNumber << std::endl;
                }

                outputFile << std::endl;
            }

            outputFile << "[Reminders]" << std::endl;
            for (const auto& reminder : reminders) {
                outputFile << reminder->getTitle() << std::endl;
                outputFile << reminder->getTime() << std::endl;
                outputFile << reminder->getExplanation() << std::endl;

                for (const auto& contact : reminder->getRelatedContacts()) {
                    outputFile << contact->getName() << " ";
                }

                outputFile << std::endl << std::endl;
            }

            outputFile.close();
        }
    }
};

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_COMMAND(ID_ADD_CONTACT, &CMainFrame::OnAddContact)
    ON_COMMAND(ID_ADD_REMINDER, &CMainFrame::OnAddReminder)
    ON_COMMAND(ID_VIEW_CONTACTS, &CMainFrame::OnViewContacts)
    ON_COMMAND(ID_VIEW_REMINDERS, &CMainFrame::OnViewReminders)
    ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL CNoteApp::InitInstance() {
    CMainFrame* frame = new CMainFrame();
    m_pMainWnd = frame;
    frame->LoadData();
    frame->ShowWindow(SW_SHOW);
    frame->UpdateWindow();
    return TRUE;
}

CNoteApp theApp;
