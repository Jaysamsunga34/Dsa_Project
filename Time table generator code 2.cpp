#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <random>
#include <vector>
#include <unordered_map>
using namespace std;

class Subject{
    public:
    string code;
    string name;
    int lectures;
    int tutorials;
    int practicals;
    int credits;
    string professor;
    int branch; 
};

class Room{
    public:
    string name;
};

class TimeSlot{
    public:
    string day;
    int starthour;
    int endhour;
    Subject subject;
    Room room;
    int lectureID; // Unique ID for each lecture
};

class TimetableGenerator{
private:
     int DAYS_IN_WEEK=5;//Assuming class to be held only for 5 days in a week
     int HOURS_IN_DAY=5; // Assuming classes from 8 to 1
    vector<Subject>subjects;
    vector<Room>rooms;
    vector<vector<vector<TimeSlot>>>timetable; // A 3D vector to store timetable

public:
    TimetableGenerator(){
        timetable.resize(DAYS_IN_WEEK,vector<vector<TimeSlot>>(HOURS_IN_DAY));
    }
    string line,roomName;;
    void readroom(string filename){
        ifstream file(filename);
        if(!file.is_open()){
            cout<<"Failed to open file: "<<filename<<endl;
            return;
        }

        while(getline(file, line)){
            if (line.empty()){
                continue;//Skipping over empty lines
            }

            stringstream ss(line);
            if(getline(ss, roomName, ',')){
                rooms.push_back({roomName});//We are taking the room names from our input file
            }
        }
        file.close();
    }

   void readsubjects(string filename){
    ifstream file(filename);
    if (!file.is_open()){
        cout<<"Failed to open file: "<<filename<<endl;
        return;
    }

 string line,code,name,lecturesStr,tutorialsStr,practicalsStr,creditsStr,professor,branchStr;
        int lectures,tutorials,practicals,credits,branch,currentBranch=0; // Tracking the current branch being read
    while(getline(file, line)){
        if(line.substr(0, 4)=="Year"){ // Skipping the lines starting with "Year"
            continue;
        }

        stringstream ss(line);
       
        if(getline(ss,code,',')&&
            getline(ss,name,',')&&
            getline(ss,lecturesStr,',')&&
            getline(ss,tutorialsStr,',')&&
            getline(ss,practicalsStr,',')&&
            getline(ss,creditsStr,',')&&
            getline(ss,professor,',')){

            lectures=stoi(lecturesStr);
            tutorials=stoi(tutorialsStr);
            practicals=stoi(practicalsStr);
            credits=stoi(creditsStr);

            //  Taking the branch information which is given after the professors name
            if(professor.back()==','){
                professor.pop_back(); // Removing  trailing comma if present
            }

            if(getline(ss,branchStr,',')){
                branch=stoi(branchStr);
            } 

            subjects.push_back({code,name,lectures,tutorials,practicals,credits,professor,branch});
        } 
        else if(!code.empty()){
            // If there is no branch information we default to the previous branch
            subjects.push_back({code,name,lectures,tutorials,practicals,credits,professor,currentBranch});
        } 
    }
    file.close();
}

void generateTimetable(){
    random_device rd;
    mt19937 g(rd());//Initialising a random number generator

     string daysOfWeek[DAYS_IN_WEEK]={"Monday","Tuesday","Wednesday","Thursday","Friday"};

    unordered_map<string, int>lecturesRemaining;
    for(Subject subject:subjects){
        int totalLectures=subject.lectures;
        lecturesRemaining[subject.code]=totalLectures;
    }

    for(int day=0;day<DAYS_IN_WEEK;++day){
        for(int hour=0;hour<HOURS_IN_DAY;++hour){
            shuffle(subjects.begin(),subjects.end(),g);
            for(Subject subject:subjects){
                shuffle(rooms.begin(),rooms.end(),g);
                if (lecturesRemaining[subject.code]>0){
                    for (Room room:rooms){
                        if (room.name!="\0"&&canAssign(subject,room,day,hour,lecturesRemaining)){
                            int lectureID=timetable[day][hour].size();
                            timetable[day][hour].push_back({daysOfWeek[day],hour + 8,hour + 9,subject,room,lectureID});
                            --lecturesRemaining[subject.code]; // Decrementing lectures for the assigned subject
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool canAssign(Subject subject,Room room,int day,int hour,unordered_map<string,int>&lecturesRemaining){
    // Taking the year information from the subject code
    char subjectYear=subject.code[2]; // The year for the course if found by the third character character
    int subjectBranch=subject.branch;

    // Taking the professor's name of the subject
    string subjectProfessor=subject.professor;

    // Checking if any lecture in the same day and hour has the same professor
    for(TimeSlot existingSlot:timetable[day][hour]){
        if(existingSlot.subject.professor==subjectProfessor){
            return false; // If there is a lecture with the same professor at the same time, return false indicating it is not possible to assign at that time
        }
    }

    // Checking if there is an  existing lecture in the same day having the same subject code
    for(int h=0;h<HOURS_IN_DAY;++h){
        for (TimeSlot existingSlot:timetable[day][h]){
            if (existingSlot.subject.code==subject.code){
                return false; // If a lecture for the subject already exists on the same day, return false
            }
        }
    }

    // Checking if any  lecture in the same day and hour has the same year and branch
    for(TimeSlot existingSlot:timetable[day][hour]){
        char existingYear=existingSlot.subject.code[2]; 
        int existingBranch=existingSlot.subject.branch;
        if (subjectYear==existingYear&&subjectBranch==existingBranch){
            return false; // If the year and branch are the same, return false
        }
    }

    // Checking if the room is available for the subject in the given day and hour
    for(TimeSlot existingSlot:timetable[day][hour]){
        if (room.name==existingSlot.room.name){
            return false; // If the room is already assigned, return false
        }
    }

    // Checking if the subject has any remaining lectures to be assigned
    if (lecturesRemaining[subject.code]<=0){
        return false;
    }

    // Checking if the room capacity is sufficient for the subject
    return true; // Here we assume that all the rooms have sufficient capacity to accomodate any subject
}

void printTimetable(){
        cout<<setw(8)<<"Day"<<setw(23)<<"Start Time"<<setw(17)<<"End Time"<<setw(21)<<"Subject Code"<<setw(20)<<"Professor"<<setw(17)<<"Room"<<endl;
        cout<<setfill('-')<<setw(110)<<"-"<<setfill(' ')<<endl;
        
         string daysOfWeek[DAYS_IN_WEEK] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};

        for(int day=0;day<DAYS_IN_WEEK;++day){
            for (int hour=0;hour<HOURS_IN_DAY;++hour){
                for(TimeSlot slot:timetable[day][hour]){
                    cout<<setw(10)<<slot.day<<setw(15)<<slot.starthour<<":00"<<setw(15)<<slot.endhour<<":00"
                        <<setw(20)<<slot.subject.code<<setw(20)<<slot.subject.professor<<setw(20)<<slot.room.name<<endl;
                }
            }
        }
    }
};

int main(){
    TimetableGenerator generator;
    generator.readroom("roomstest.csv");
    generator.readsubjects("subjects.csv");//Reading input from the files
    generator.generateTimetable();
    generator.printTimetable();
    return 0;
} 