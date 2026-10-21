#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

#define FILENAME "contacts.csv"
#define EXPORT_FILENAME "telephone_directory_export.csv"

// ==========================================
// 1. DATA STRUCTURE: BINARY SEARCH TREE
// ==========================================
typedef struct BSTNode {
    char name[30];
    char phone[15];
    char category[15];
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

BSTNode* createNode(const char* name, const char* phone, const char* category) {
    BSTNode* newNode = (BSTNode*)malloc(sizeof(BSTNode));
    strcpy(newNode->name, name);
    strcpy(newNode->phone, phone);
    strcpy(newNode->category, category);
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

BSTNode* insert(BSTNode* root, const char* name, const char* phone, const char* category) {
    if (root == NULL) return createNode(name, phone, category);
    
    if (strcasecmp(name, root->name) < 0)
        root->left = insert(root->left, name, phone, category);
    else if (strcasecmp(name, root->name) > 0)
        root->right = insert(root->right, name, phone, category);
    else {
        strcpy(root->phone, phone);
        strcpy(root->category, category);
    }
    return root;
}

BSTNode* searchNode(BSTNode* root, const char* name) {
    if (root == NULL || strcasecmp(root->name, name) == 0) return root;
    if (strcasecmp(name, root->name) < 0) return searchNode(root->left, name);
    return searchNode(root->right, name);
}

BSTNode* findMin(BSTNode* root) {
    while (root && root->left != NULL) root = root->left;
    return root;
}

BSTNode* deleteNode(BSTNode* root, const char* name, int *found) {
    if (root == NULL) return NULL;

    if (strcasecmp(name, root->name) < 0) {
        root->left = deleteNode(root->left, name, found);
    } else if (strcasecmp(name, root->name) > 0) {
        root->right = deleteNode(root->right, name, found);
    } else {
        *found = 1;
        if (root->left == NULL) {
            BSTNode* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            BSTNode* temp = root->left;
            free(root);
            return temp;
        }
        BSTNode* temp = findMin(root->right);
        strcpy(root->name, temp->name);
        strcpy(root->phone, temp->phone);
        strcpy(root->category, temp->category);
        root->right = deleteNode(root->right, temp->name, found);
    }
    return root;
}

int getCount(BSTNode* root) {
    if (root == NULL) return 0;
    return 1 + getCount(root->left) + getCount(root->right);
}

int getDepth(BSTNode* root) {
    if (root == NULL) return 0;
    int l = getDepth(root->left);
    int r = getDepth(root->right);
    return 1 + (l > r ? l : r);
}

void freeTree(BSTNode* root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

// ==========================================
// 2. FILE PERSISTENCE & EXCEL EXPORT
// ==========================================
void writeTreeToCSV(BSTNode* root, FILE* fp) {
    if (root != NULL) {
        writeTreeToCSV(root->left, fp);
        fprintf(fp, "%s,%s,%s\n", root->name, root->phone, root->category);
        writeTreeToCSV(root->right, fp);
    }
}

void saveToCSV(BSTNode* root) {
    FILE* fp = fopen(FILENAME, "w");
    if (fp != NULL) {
        writeTreeToCSV(root, fp);
        fclose(fp);
    }
}

BSTNode* loadFromCSV() {
    BSTNode* root = NULL;
    FILE* fp = fopen(FILENAME, "r");
    if (fp == NULL) return NULL;

    char line[100];
    while (fgets(line, sizeof(line), fp)) {
        char name[30], phone[15], category[15];
        char* token = strtok(line, ",\n");
        if (token) strcpy(name, token);
        token = strtok(NULL, ",\n");
        if (token) strcpy(phone, token);
        token = strtok(NULL, ",\n");
        if (token) strcpy(category, token);

        if (strlen(name) > 0 && strlen(phone) > 0) {
            root = insert(root, name, phone, strlen(category) > 0 ? category : "General");
        }
    }
    fclose(fp);
    return root;
}

// Write contacts in sorted order with formal Excel headers
void writeExcelRow(BSTNode* root, FILE* fp) {
    if (root != NULL) {
        writeExcelRow(root->left, fp);
        fprintf(fp, "\"%s\",\"%s\",\"%s\"\n", root->name, root->phone, root->category);
        writeExcelRow(root->right, fp);
    }
}

int exportToExcel(BSTNode* root) {
    FILE* fp = fopen(EXPORT_FILENAME, "w");
    if (fp == NULL) return 0;

    // Excel Column Headers
    fprintf(fp, "Name,Phone Number,Category\n");
    writeExcelRow(root, fp);
    fclose(fp);
    return 1;
}

// ==========================================
// 3. FRONTEND & GRAPHICS ENGINE
// ==========================================
void drawTree(BSTNode* root, int x, int y, int hOffset, int isRoot, const char* highlightName) {
    if (root == NULL) return;

    int nextY = y + 65;

    if (root->left != NULL) {
        setcolor(LIGHTCYAN);
        setlinestyle(SOLID_LINE, 0, 2);
        line(x, y, x - hOffset, nextY);
        drawTree(root->left, x - hOffset, nextY, hOffset / 2, 0, highlightName);
    }

    if (root->right != NULL) {
        setcolor(LIGHTCYAN);
        setlinestyle(SOLID_LINE, 0, 2);
        line(x, y, x + hOffset, nextY);
        drawTree(root->right, x + hOffset, nextY, hOffset / 2, 0, highlightName);
    }

    // Drop Shadow
    setfillstyle(SOLID_FILL, DARKGRAY);
    fillellipse(x + 3, y + 3, 32, 19);

    int bgFill = CYAN;
    int fgText = BLACK;

    if (highlightName && strlen(highlightName) > 0 && strcasecmp(root->name, highlightName) == 0) {
        bgFill = RED;
        fgText = WHITE;
    } else if (isRoot) {
        bgFill = YELLOW;
        fgText = BLACK;
    } else {
        bgFill = CYAN;
        fgText = BLACK;
    }

    setfillstyle(SOLID_FILL, bgFill);
    fillellipse(x, y, 32, 19);
    setcolor(WHITE);
    setlinestyle(SOLID_LINE, 0, 1);
    ellipse(x, y, 0, 360, 32, 19);

    setbkcolor(bgFill);
    setcolor(fgText);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);

    char label[10];
    strncpy(label, root->name, 6);
    label[6] = '\0';

    int tw = textwidth(label);
    int th = textheight(label);
    outtextxy(x - (tw / 2), y - (th / 2), label);
}

void displayInorder(BSTNode* root, int *yOffset, int *index) {
    if (root != NULL && *index <= 12) {
        displayInorder(root->left, yOffset, index);

        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(575, *yOffset - 3, 865, *yOffset + 22);
        setcolor(CYAN);
        rectangle(575, *yOffset - 3, 865, *yOffset + 22);

        char entry[80];
        sprintf(entry, "%2d. %-8s | %s", (*index)++, root->name, root->phone);
        
        setbkcolor(DARKGRAY);
        setcolor(YELLOW);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        outtextxy(582, *yOffset + 3, entry);
        *yOffset += 28;

        displayInorder(root->right, yOffset, index);
    }
}

void renderUI(BSTNode* root, const char* statusMsg, int isSuccess, const char* highlightName) {
    // 1. Header Banner
    setfillstyle(SOLID_FILL, BLUE);
    bar(0, 0, 880, 50);
    setbkcolor(BLUE);
    setcolor(YELLOW);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3);
    outtextxy(200, 8, (char*)"TELEPHONE DIRECTORY BST");

    // 2. Stats Dashboard
    char cStr[30], dStr[30];
    sprintf(cStr, " Contacts: %d ", getCount(root));
    sprintf(dStr, " BST Height: %d ", getDepth(root));

    setfillstyle(SOLID_FILL, MAGENTA);
    bar(15, 60, 150, 88);
    setbkcolor(MAGENTA);
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(25, 68, cStr);

    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(160, 60, 290, 88);
    setbkcolor(DARKGRAY);
    setcolor(WHITE);
    outtextxy(170, 68, dStr);

    // 3. Left Panel (BST Viewport)
    setcolor(CYAN);
    setlinestyle(SOLID_LINE, 0, 2);
    rectangle(10, 55, 560, 480);
    
    if (root != NULL) {
        drawTree(root, 285, 120, 125, 1, highlightName);
    } else {
        setbkcolor(BLACK);
        setcolor(LIGHTGRAY);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
        outtextxy(200, 240, (char*)"[ Directory Empty ]");
    }

    // 4. Right Panel (Sorted List)
    setcolor(LIGHTGREEN);
    rectangle(570, 55, 870, 480);
    
    setfillstyle(SOLID_FILL, GREEN);
    bar(571, 56, 869, 88);
    setbkcolor(GREEN);
    setcolor(BLACK);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
    outtextxy(620, 64, (char*)"SORTED DIRECTORY");

    int yOffset = 102;
    int index = 1;
    displayInorder(root, &yOffset, &index);

    // 5. Interactive Action Buttons (5 Buttons Layout)
    int btnX1[] = {10, 180, 350, 520, 690};
    int btnX2[] = {170, 340, 510, 680, 850};
    const char* btnLabels[] = {"+ ADD", "? SEARCH", "- DELETE", "^ EXPORT", "x EXIT"};

    for (int i = 0; i < 5; i++) {
        setfillstyle(SOLID_FILL, BLUE);
        bar(btnX1[i], 492, btnX2[i], 532);
        setcolor(YELLOW);
        setlinestyle(SOLID_LINE, 0, 1);
        rectangle(btnX1[i], 492, btnX2[i], 532);
        
        setbkcolor(BLUE);
        setcolor(WHITE);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        outtextxy(btnX1[i] + 15, 507, (char*)btnLabels[i]);
    }

    // 6. Dynamic Status Alert Banner
    if (strlen(statusMsg) > 0) {
        int bannerColor = isSuccess ? GREEN : RED;
        setfillstyle(SOLID_FILL, bannerColor);
        bar(10, 542, 870, 582);
        setcolor(WHITE);
        rectangle(10, 542, 870, 582);
        
        setbkcolor(bannerColor);
        setcolor(WHITE);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        outtextxy(25, 557, (char*)statusMsg);
    }
}

// ==========================================
// 4. IN-WINDOW INPUT MODAL
// ==========================================
void getGUIInput(const char* title, const char* prompt, char* outputBuffer, int maxLen) {
    outputBuffer[0] = '\0';
    int idx = 0;

    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(180, 180, 700, 370);
    setcolor(YELLOW);
    setlinestyle(SOLID_LINE, 0, 2);
    rectangle(180, 180, 700, 370);

    setbkcolor(DARKGRAY);
    setcolor(CYAN);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    outtextxy(200, 195, (char*)title);

    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(200, 245, (char*)prompt);

    while (1) {
        setfillstyle(SOLID_FILL, BLACK);
        bar(200, 275, 680, 315);
        setcolor(YELLOW);
        setlinestyle(SOLID_LINE, 0, 1);
        rectangle(200, 275, 680, 315);

        setbkcolor(BLACK);
        setcolor(GREEN);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
        outtextxy(210, 288, outputBuffer);

        char ch = getch();
        if (ch == 13) {
            break;
        } else if (ch == 8) {
            if (idx > 0) {
                idx--;
                outputBuffer[idx] = '\0';
            }
        } else if (isprint(ch) && idx < maxLen - 1) {
            outputBuffer[idx++] = ch;
            outputBuffer[idx] = '\0';
        }
    }
}

// ==========================================
// 5. MAIN PROGRAM LOOP
// ==========================================
int main() {
    initwindow(880, 600, (char*)"Advanced Phonebook Engine - BST Project");

    BSTNode* root = loadFromCSV();
    if (root == NULL) {
        root = insert(root, "Rahul", "9876543210", "Friend");
        root = insert(root, "Amit", "9123456789", "Work");
        root = insert(root, "Suresh", "9988776655", "Family");
        root = insert(root, "Priya", "9456123789", "Work");
        saveToCSV(root);
    }

    char statusMsg[100] = "System Ready. Click buttons or hotkeys (1-5).";
    char searchHighlight[30] = "";
    int isSuccess = 1;
    int page = 0;

    while (1) {
        setactivepage(page);
        cleardevice();

        renderUI(root, statusMsg, isSuccess, searchHighlight);

        setvisualpage(page);

        int action = 0; // 1: ADD, 2: SEARCH, 3: DELETE, 4: EXPORT, 5: EXIT

        if (ismouseclick(WM_LBUTTONDOWN)) {
            int mx, my;
            getmouseclick(WM_LBUTTONDOWN, mx, my);

            if (my >= 492 && my <= 532) {
                if (mx >= 10 && mx <= 170)       action = 1; // ADD
                else if (mx >= 180 && mx <= 340) action = 2; // SEARCH
                else if (mx >= 350 && mx <= 510) action = 3; // DELETE
                else if (mx >= 520 && mx <= 680) action = 4; // EXPORT TO EXCEL
                else if (mx >= 690 && mx <= 850) action = 5; // EXIT
            }
        }

        if (kbhit()) {
            char choice = getch();
            if (choice == '1') action = 1;
            else if (choice == '2') action = 2;
            else if (choice == '3') action = 3;
            else if (choice == '4' || choice == 'e' || choice == 'E') action = 4;
            else if (choice == '5' || choice == 'q' || choice == 'Q') action = 5;
        }

        if (action == 1) {
            char name[30], phone[15], category[15];
            getGUIInput("ADD NEW CONTACT", "Enter Contact Name:", name, 28);
            if (strlen(name) > 0) {
                getGUIInput("ADD NEW CONTACT", "Enter Phone Number:", phone, 14);
                getGUIInput("ADD NEW CONTACT", "Enter Category (Work/Family/Friend):", category, 14);

                root = insert(root, name, phone, strlen(category) > 0 ? category : "General");
                saveToCSV(root);
                sprintf(statusMsg, "SUCCESS: Contact '%s' added & saved!", name);
                isSuccess = 1;
            }
            searchHighlight[0] = '\0';
        }
        else if (action == 2) {
            char query[30];
            getGUIInput("SEARCH CONTACT", "Enter Name to Search:", query, 28);
            if (strlen(query) > 0) {
                BSTNode* res = searchNode(root, query);
                if (res != NULL) {
                    sprintf(statusMsg, "FOUND: %s -> Phone: %s [%s]", res->name, res->phone, res->category);
                    strcpy(searchHighlight, res->name);
                    isSuccess = 1;
                } else {
                    sprintf(statusMsg, "ERROR: Contact '%s' was not found.", query);
                    searchHighlight[0] = '\0';
                    isSuccess = 0;
                }
            }
        }
        else if (action == 3) {
            char query[30];
            int found = 0;
            getGUIInput("DELETE CONTACT", "Enter Name to Delete:", query, 28);
            if (strlen(query) > 0) {
                root = deleteNode(root, query, &found);
                if (found) {
                    saveToCSV(root);
                    sprintf(statusMsg, "SUCCESS: Contact '%s' deleted from BST!", query);
                    isSuccess = 1;
                } else {
                    sprintf(statusMsg, "ERROR: Contact '%s' not found to delete.", query);
                    isSuccess = 0;
                }
            }
            searchHighlight[0] = '\0';
        }
        else if (action == 4) { // EXPORT TO EXCEL
            if (exportToExcel(root)) {
                sprintf(statusMsg, "EXCEL EXPORT SUCCESS: Saved to '%s'", EXPORT_FILENAME);
                isSuccess = 1;
            } else {
                sprintf(statusMsg, "EXCEL EXPORT ERROR: Failed to write file.");
                isSuccess = 0;
            }
            searchHighlight[0] = '\0';
        }
        else if (action == 5) { // EXIT
            saveToCSV(root);
            break;
        }

        page = 1 - page;
        delay(80);
    }

    freeTree(root);
    closegraph();
    return 0;
}