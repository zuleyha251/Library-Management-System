#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
// Sabit tanımlamaları
#define MAX_TITLE_LENGTH 100//kitap başlığı için max karakter
#define MAX_AUTHOR_LENGTH 100//yazar adı için max karakteri belirler
#define MAX_ISBN_LENGTH 20
#define MAX_CATEGORY_LENGTH 50
#define MAX_NAME_LENGTH 100
#define MAX_USERNAME_LENGTH 50//kullanıcı adı için max uzunluk belirler.
#define MAX_PASSWORD_LENGTH 50
#define MAX_BOOKS 500
#define MAX_USERS 100
#define MAX_BORROWED_BOOKS 3//en fazla ödünç alınabilecek kitap sayısı
#define LOAN_PERIOD 15//odunc suresi
#define BOOK_FILE "kitaplar.bin"//kitapların kaydedildiği dosya
#define USER_FILE "kullanicilar.bin"//kullanıcıların kaydedildiği dosya

typedef enum// Kitap durumu için enum
{
    AVAILABLE = 'A',
    BORROWED = 'B'
} BookStatus;

typedef enum// Kullanıcı rolü için enum
{
    ADMIN = 'A',
    USER = 'U'
} UserRole;

typedef struct// Kitap yapısı (struct)
{
    int bookId;
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char isbn[MAX_ISBN_LENGTH];
    char category[MAX_CATEGORY_LENGTH];
    BookStatus status;
    time_t borrowdate;//ödünç veme tarihi
    time_t returndate;//iade tarihi
} Book;

typedef struct// Kullanıcı yapısı (struct)
{
    int userId;
    char name[MAX_NAME_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    UserRole role;
    int borrowedBooks[MAX_BORROWED_BOOKS];//ödünç aldığı kitapların idlerini tutar.
    int numBorrowedBooks; //ödünç alınan kitap sayısını tutar
} User;

// Global değişkenler 
Book books[MAX_BOOKS];//Book structının bilgilerini içeren dizi
int numBooks = 0;
User users[MAX_USERS];//User structının bilgilerini içeren dzi
int numUsers = 0;

// Fonksiyon prototipleri 
void addBook();
void manageBook();
void listAllofBooks();
Book *findBookById(int bookId);
void listBorrowedBooks(User *loggedUser);
void searchBookwithTitle();
void searchBookByCategory();
void borrowBook(User *loggedUser);
void borrowBookByTitle(User *loggedUser); 
void returnBook(User *loggedUser);
void checkOverdueBooks(User *loggedUser);
char* formatTime(time_t rawTime);

void addUser();
void listAllusers();
User *userAuthentication();
User *findUserById(int userId);

void displayAdminMenu(User *currentUser);
void displayUserMenu(User *currentUser);
void displayLoginMenu();

int loadBooksFromFile(const char *filenameKitap, Book *books);
void saveBooksToFile(const char *filenameKitap, const Book *books, int count);
int loadUsersFromFile(const char *filenamekullanici, User *users);
void saveUsersToFile(const char *filenamekullanici, const User *users, int count);

char *formatTime(time_t rawTime)
{
    static char timeStr[100];//programın ömrü boyunca varligini surdurur ve yalnızca bir kez başlatılır. Fonksiyon her cagrıldığında yeniden oluşturulmaz.
    struct tm *timeInfo = localtime(&rawTime);//rawTime (time_t tipinde) tarafından temsil edilen zamanı, yerel saat dilimine göre bir struct tm yapısına donusturur.
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeInfo);
    //strftime() (string format time) fonksiyonu, struct tm yapısındaki zaman bilgilerini belirli bir format dizesine göre bir karakter dizisine (string'e) dönüştürür.
    //timeStr: Formatlanmış zaman dizesinin yazılacağı hedef arabellek.
    //sizeof(timeStr): Hedef arabelleğin maksimum boyutu. Bu, arabellek taşmasını önlemeye yardımcı olur
    return timeStr;
}

// Kitap ID'sine göre kitabı bulan fonksiyon
Book* findBookById(int bookId)
{
    for(int i = 0; i < numBooks; i++) 
    {
        if(books[i].bookId == bookId)//aranan Id ile kayıtlı olan Id eşleşmesini kontrol eder.
        {
            return &books[i];//eşleşme olursa girilen Id'yi döndürür.
        }
    }
    return NULL;
}

User* findUserById(int userId)// Kullanıcı ID'sine göre kullanıcıyı bulan fonksiyon
{
    for(int i = 0; i < numUsers; i++) 
    {
        if(users[i].userId == userId)//girilen kullanıcı Id'si ile kayıtlı olan Id eşleşmesini kontrol eder.
        {
            return &users[i];//eşleşme girilen Id'ye sahip kullaniciyi dondurur.
        }
    }
    return NULL;
}

// books dizisinde tutulan tüm bilgileri dosyaya kaydeder.
void saveBooksToFile(const char *filenameKitap, const Book *books, int numBooks) {
    FILE *bookfile = fopen(filenameKitap, "wb");
    if (bookfile == NULL) {
        perror("Dosya acma hatasi");
        return;
    }
    fwrite(books, sizeof(Book), numBooks, bookfile);//bookfile'ın işaret ettiği dosyaya numbooks*sizeof kadarlık byteı books başlangıç adresinden başlayarak kaydeder.
    fclose(bookfile);
    printf("Kitap bilgileri kaydedildi.\n");
}

// Kitap bilgilerini dosyadan yukleme fonksiyonu 
int loadBooksFromFile(const char *filenameKitap, Book *books) {
    FILE *bookfile = fopen(filenameKitap, "rb");//kitaplarin kaydedidigi dosyayi belirten pointer ile rb modunda acar.
    if (bookfile == NULL) {
        printf("Dosya acma hatasi (yeni dosya olusturulabilir)\n");
        return 0;
    }
    int count = fread(books, sizeof(Book), MAX_BOOKS, bookfile);//bookfile'in isaret ettigi dosyaya sizeof*max_books kadarlik btye veriyi books başlangıç adresinden başlayarak kaydeder.
    fclose(bookfile);
    printf("%d kitap bilgisi yuklendi.\n", count);
    return count;
}

// Kullanici bilgilerini dosyaya kaydetme fonksiyonu.
void saveUsersToFile(const char *filenamekullanici, const User *users, int count) {
    FILE *userfile = fopen(filenamekullanici, "wb");
    if (userfile == NULL) {
        perror("Dosya acma hatasi");
        return;
    }
    fwrite(users, sizeof(User), count, userfile);
    fclose(userfile);
    printf("Kullanici bilgileri kaydedildi.\n");
}

// Kullanıcı bilgilerini dosyadan yükleme fonksiyonu.
int loadUsersFromFile(const char *filenamekullanici, User *users) {
    FILE *userfile = fopen(filenamekullanici, "rb");
    if (userfile == NULL) {
        printf("Dosya açma hatasi (yeni dosya oluşturulabilir)\n");
        return 0;
    }
    int count = fread(users, sizeof(User), MAX_USERS, userfile);
    fclose(userfile);
    printf("%d kullanici bilgisi yüklendi.\n", count);
    return count;
}

void addBook()
{
    if (numBooks < MAX_BOOKS){
        Book newBook;//Book structının elemanlarını içeren ve girilen bilgileri tutacak olan dizi.

        newBook.bookId = numBooks + 1;//basit bir Id oluşturucu.
        printf("Kitap Basligi : ");
        fgets(newBook.title, MAX_TITLE_LENGTH, stdin);
        newBook.title[strcspn(newBook.title, "\n")] = 0;
        printf("Kitap Yazari : ");
        fgets(newBook.author, MAX_AUTHOR_LENGTH, stdin);
        newBook.author[strcspn(newBook.author, "\n")] = 0;
        printf("Kategori(Bilim Kurgu/Gerilim/Biyografi/Polisiye/Roman/Felsefe/Hikaye) : ");
        fgets(newBook.category, MAX_CATEGORY_LENGTH, stdin);
        newBook.category[strcspn(newBook.category, "\n")] = 0;
        printf("ISBN : ");
        fgets(newBook.isbn, MAX_ISBN_LENGTH, stdin);
        newBook.isbn[strcspn(newBook.isbn, "\n")] = 0; //fgets ile gelebilecek olan new line karakterini sildirir ve yerine 
        newBook.status = AVAILABLE;
        newBook.borrowdate = 0;
        newBook.returndate = 0;

        books[numBooks] = newBook; //girilen bilgileri tutan newBook dizisini asıl books dizisine kaydeder. 
        numBooks++; //kitap sayısını artırır.
        printf("Yeni Kitap Eklendi (ID = %d).\n", newBook.bookId);
        saveBooksToFile(BOOK_FILE, books, numBooks); 
    }
    else{
        printf("Maksimum kitap sayisina ulasildi.\n");
    }
}


void manageBook()//Kitap bilgilerini güncelleyen fonksiyon
{
    if(numBooks==0){
        printf("Sistemde kitap bulunmamaktadir.\n");
        return;
    }

    int bookIdToManage;

    printf("-----Kitap Yonetim-----\n");
    printf("Islem yapmak istediginiz kitabin ID'sini giriniz: ");
    scanf("%d", &bookIdToManage);
    getchar();

    Book *bookToManage = findBookById(bookIdToManage);//book yapısını işaret eden pointera id ile kitabı bulan fonkisyon sonucunu atadım.

    if(bookToManage == NULL) {
        printf("Belirtilen ID'ye sahip kitap bulunamamistir.\n");
        return;
    }

    printf("\nKitap Bilgileri:\n");
    printf("ID: %d, Baslik: %s, Yazar: %s, ISBN: %s, Kategori: %s, Durum: %c\n",
           bookToManage->bookId, bookToManage->title, bookToManage->author,
           bookToManage->isbn, bookToManage->category, bookToManage->status);
    if (bookToManage->borrowdate != 0) {//ödünç alınmışsa kitap üzerinde değişiklik yapmaz. 
        printf("Odunc Alma Tarihi: %s\n", formatTime(bookToManage->borrowdate));
        printf("Iade Tarihi: %s\n", formatTime(bookToManage->returndate));
    }
    printf("--------------------------\n");

    int choice;
    printf("Yapmak istediginiz islemi secin:\n");
    printf("1. Kitap Bilgilerini Guncelle\n");
    printf("2. Kitabi Sil\n");
    printf("0. Islemi Iptal Et\n");
    printf("Seciminiz: ");
    scanf("%d", &choice);
    getchar(); // newline karakterini temizle

    switch (choice)
    {
    case 1:
        printf("-----Kitap Guncelle-----\n");
        printf("Yeni Baslik (guncellemek istemiyorsaniz Enter tuslayin.): ");
        char newTitle[MAX_TITLE_LENGTH];
        fgets(newTitle, MAX_TITLE_LENGTH, stdin);
        newTitle[strcspn(newTitle, "\n")] = 0;
        if(strlen(newTitle) > 0) {
           strcpy(bookToManage->title, newTitle);
           printf("Baslik guncellendi.\n");
        }

        printf("Yeni Kategori (guncellemek istemiyorsaniz Enter tuslayin.): ");
        char newCategory[MAX_CATEGORY_LENGTH];//kullanıcının gireceği kategori bilgisini tutan dizi.
        fgets(newCategory, MAX_CATEGORY_LENGTH, stdin);
        newCategory[strcspn(newCategory, "\n")] = 0;
        if(strlen(newCategory) > 0) {
            strcpy(bookToManage->category, newCategory);//yeni kategori bilgisini eski bilginin yerine yazar.
            printf("Kategori guncellendi.\n");
        }

        printf("Yeni Yazar (guncellemek istemiyorsaniz Enter tuslayin.): ");
        char newAuthor[MAX_AUTHOR_LENGTH];
        fgets(newAuthor, MAX_AUTHOR_LENGTH, stdin);
        newAuthor[strcspn(newAuthor, "\n")] = 0;
        if(strlen(newAuthor) > 0) {
            strcpy(bookToManage->author, newAuthor);
            printf("Yazar güncellendi.\n");
        }

        printf("Yeni ISBN (Degistirmek istemiyorsaniz Enter'a basin): ");
            char newIsbn[MAX_ISBN_LENGTH];
            fgets(newIsbn, MAX_ISBN_LENGTH, stdin);
            newIsbn[strcspn(newIsbn, "\n")] = 0;
            if (strlen(newIsbn) > 0) {
                strcpy(bookToManage->isbn, newIsbn);
                printf("ISBN guncellendi.\n");
            }
            printf("Kitap bilgileri basariyla guncellendi.\n");
            saveBooksToFile(BOOK_FILE, books, numBooks);//yapılan değişikleri dosyaya kaydediyor.
            
        break;

    case 2:
        if(bookToManage->status == BORROWED){
            printf("Kitap su an odunc verildigi icin silme islemi gerceklestirilemez.\n");
            break;
        }

        // Kitabı diziden silme (elemanları kaydırma)
        int deletedBookIndex = -1;//silinecek kitabın konumunu tutacaktır.-1 olmasının nedeni kitabın henüz bulunamamasıdır.
        for (int i = 0; i < numBooks; i++)  {
            if (books[i].bookId == bookIdToManage)//her kitap için silinmek istenen kitabın Id'si ile kayıtlı kitapları karşılaştırır.
            {
                deletedBookIndex = i;//eşleşen kitap olduğunda onun indeksini atar.
                break;
            }
        }

        if (deletedBookIndex != -1) {
            char deletedTitle[MAX_TITLE_LENGTH];//local olarak luşturulan bu dizi silinecek kitabın başlığını tutar.
            strcpy(deletedTitle, books[deletedBookIndex].title);

            for (int i = deletedBookIndex; i < numBooks - 1; i++) {///silincek kitaptan sonraki tüm kitapları bir önceki konuma kaydeder.
                books[i] = books[i + 1];
            }
            numBooks--;
            printf("'%s' adli kitap sistemden silindi.\n", deletedTitle);
            saveBooksToFile(BOOK_FILE, books, numBooks);
        } 
        else  {
            printf("Kitap silinirken bir hata olustu. Kitap dizide bulunamadi.\n");
        }
        break;
        
    case 0:
        printf("Islem iptal edildi.\n");
        break;
        
    default:
        printf("Gecersiz islem.\n");
        break;
    }
}


void listAllofBooks()
{
    if (numBooks > 0) {
        printf("\n----Kitaplar----\n");
        for (int i = 0; i < numBooks; i++) {
            printf("ID:%d, Baslik:%s, Yazar:%s, ISBN:%s, Kategori:%s, Durum:%c \n",
                   books[i].bookId, books[i].title, books[i].author, books[i].isbn,
                   books[i].category, books[i].status);
            printf("Odunc Alma Tarihi:%s , Iade Tarihi : %s\n",
                   (books[i].borrowdate == 0) ? "Yok" : formatTime(books[i].borrowdate),
                   (books[i].returndate == 0) ? "Yok" : formatTime(books[i].returndate));       
        }
        printf("------------------\n");
    }
    else {
        printf("Kutuphanede kitap bulunmamaktadir.\n");
    }
}

void addUser()
{
    if (numUsers < MAX_USERS)
    {
        User newUser;
        newUser.userId = numUsers + 1;
        printf("Kullanici Adi : ");
        fgets(newUser.username, MAX_USERNAME_LENGTH, stdin);
        newUser.username[strcspn(newUser.username, "\n")] = 0;
        printf("Parola : ");
        fgets(newUser.password, MAX_PASSWORD_LENGTH, stdin);
        newUser.password[strcspn(newUser.password, "\n")] = 0;
        printf("Ad Soyad : ");
        fgets(newUser.name, MAX_NAME_LENGTH, stdin);
        newUser.name[strcspn(newUser.name, "\n")] = 0;
        newUser.role = USER;

        newUser.numBorrowedBooks = 0;
        for (int i = 0; i < MAX_BORROWED_BOOKS; i++) {
            newUser.borrowedBooks[i] = 0;
        }
        users[numUsers] = newUser;
        numUsers++;
        printf("Yeni kullanici eklendi (ID=%d).\n", newUser.userId);
        saveUsersToFile(USER_FILE, users, numUsers);
    }
    else {
        printf("Maksimum kullanici sayisina ulasildigindan kullanici eklenememistir.\n");
    }
}


void searchUserByName()
{
    char searchName[MAX_NAME_LENGTH];
    int found = 0;//aramada kullanıcı bulunup bulunmadığını tutar.

    printf("Uyenin adini ve soyadini giriniz: ");
    fgets(searchName, MAX_NAME_LENGTH, stdin);
    searchName[strcspn(searchName, "\n")] = 0;

    printf("\n-----Arama Sonuclari (Ad Soyada Gore)-----\n");
    for (int i = 0; i < numUsers; i++) {
        if (strstr(users[i].name, searchName) != NULL)//strstr fonksiyonu ile searchName değişkenini users[i].name içerisinde ararız ve geçtiği ilk indeksi tutarız.
        { 
            printf("ID: %d, Ad Soyad: %s, Kullanici Adi: %s, Rol: %c\n",
                   users[i].userId, users[i].name, users[i].username, users[i].role);
            found = 1;
        }
    }

    if (!found)//found hala 0 ise
    {
        printf("Belirtilen ada sahip uye bulunamadi.\n");
    }
    printf("-------------------------------------------\n");
}

void listAllusers()
{
    if (numUsers > 0){
        printf("\n-----Kullanicilar-----\n"); 
        for (int i = 0; i < numUsers; i++){
            printf("ID: %d, Ad Soyad: %s, Kullanici Adi: %s, Rol: %c \n",
                   users[i].userId, users[i].name, users[i].username, users[i].role);
        }
        printf("----------------------\n");
    }
    else{
        printf("Sistemde kullanici bulunamamistir.\n");
    }
}

User *userAuthentication()
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    User *foundUser = NULL;//User structını işaret eden bir pointer

    printf("Kullanici Adi : ");
    fgets(username, MAX_USERNAME_LENGTH, stdin);
    username[strcspn(username, "\n")] = 0;
    printf("Parola : ");
    fgets(password, MAX_PASSWORD_LENGTH, stdin);
    password[strcspn(password, "\n")] = 0;

    for (int i = 0; i < numUsers; i++){
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)//sisteme girilen kullanıcı adı ve şifreyi dizi ile karşılaştırır.
        {
            foundUser = &users[i];//eğer eşleşme tutarsa bellek adresini foundUser' a atar.
            break;
        }
    }

    if (foundUser == NULL) {
        printf("Kimlik dogrulama basarisiz. Kullanici adi veya parola hatali.\n");
    } else {
        printf("Giris basarili! Hos geldiniz, %s.\n", foundUser->name);
    }
    return foundUser;
}

void searchBookwithTitle()
{
    char titleOfBook[MAX_TITLE_LENGTH];
    int found = 0;

    printf("Aramak istediginiz kitabin basligini giriniz : ");
    fgets(titleOfBook, MAX_TITLE_LENGTH, stdin);
    titleOfBook[strcspn(titleOfBook, "\n")] = 0;

    printf("\n-----Arama Sonuclari (Basliga Gore)-----\n");
    for (int i = 0; i < numBooks; i++) {
        if (strstr(books[i].title, titleOfBook) != NULL){
            printf("ID: %d, Baslik: %s, Yazar: %s, ISBN: %s, Kategori: %s, Durum: %c\n",
                   books[i].bookId, books[i].title, books[i].author, books[i].isbn,
                   books[i].category, books[i].status);
            found = 1;
        }
    }

    if (found == 0) {
        printf("Aradiginiz kitap bulunamamistir.\n");
    }
    printf("----------------------------------------\n");
}

void searchBookByCategory()
{
    char searchCategory[MAX_CATEGORY_LENGTH]; 

    printf("Aranacak Kategori(Bilim Kurgu/Felsefe/Biyografi/Hikaye/Roman/Gerilim): ");
    fgets(searchCategory, MAX_CATEGORY_LENGTH, stdin);
    searchCategory[strcspn(searchCategory, "\n")] = 0;

    printf("\n--- Arama Sonuclari (Kategoriye Gore) ---\n");

    int found = 0;
    for (int i = 0; i < numBooks; i++)
    {
        if (strcmp(books[i].category, searchCategory) == 0) { 
            printf("ID: %d, Baslik: %s, Yazar: %s, ISBN: %s, Kategori: %s, Durum: %c\n",
                   books[i].bookId, books[i].title, books[i].author, books[i].isbn,
                   books[i].category, books[i].status);
            found = 1;
        }
    }

    if (!found)  {
        printf("Belirtilen kategoriye ait kitap bulunamadi.\n");
    }
    printf("------------------------------------------\n");
}

void borrowBook(User *loggedUser)//User structını işaret eden bir pointer
{
    if (loggedUser == NULL){
        printf("Lutfen giris yapiniz.\n");
        return;
    }

    if (loggedUser->numBorrowedBooks >= MAX_BORROWED_BOOKS) {
        printf("Maksimum odunc alma sinirina ulastiniz (%d kitap).\n", MAX_BORROWED_BOOKS); 
        return;
    }

    int bookId;
    printf("Odunc almak istediginiz kitabin ID'sini girin: ");
    scanf("%d", &bookId);
    getchar();//new line karakterini temizler.

    Book *bookToBorrow = findBookById(bookId);//findBookById fonksiyonu ile bulunan kitabı pointer a atar.
   
    if (bookToBorrow == NULL) {
        printf("Belirtilen ID'ye sahip kitap bulunamadi.\n");
        return;
    }

    if (bookToBorrow->status == BORROWED) {
        printf("Bu kitap zaten odunc alinmis durumda.\n");
        return;
    }

    bookToBorrow->status = BORROWED;
    bookToBorrow->borrowdate = time(NULL);//sistemin zamanını time_t değerinde döndürür.
    time_t returnDuration = 15*24*60*60;//15 gün * 24 saat * 60dk * 60sn  işlemiyle kitabın son teslim tarihini (saniye cinsinde) hesaplar.
    bookToBorrow->returndate = bookToBorrow->borrowdate + returnDuration;//ödünç aldiği tarihe hasaplamayı ekler 
   
    loggedUser->borrowedBooks[loggedUser->numBorrowedBooks] = bookToBorrow->bookId;//ödünç alınacak kitabın id'sini sonraki boş indekse ekler.
    loggedUser->numBorrowedBooks++;
    
    printf("'%s' adli kitap, %s tarafindan odunc alindi.\n", bookToBorrow->title, loggedUser->name);
    printf("Iade tarihi: %s\n", formatTime(bookToBorrow->returndate));
    
    saveBooksToFile(BOOK_FILE, books, numBooks);
    saveUsersToFile(USER_FILE, users, numUsers);
}

void borrowBookByTitle(User* loggedUser) {//loggedUser User structını işaret eden bir pointer
    if (loggedUser == NULL) {
        printf("Lutfen giris yapiniz.\n");
        return;
    }
    if (loggedUser->numBorrowedBooks >= MAX_BORROWED_BOOKS) {
        printf("Maksimum odunc alma sinirina ulastiniz (%d kitap).\n", MAX_BORROWED_BOOKS);
        return;//User structının elemanından olan ödünç alınan kitap sayısı sınırı geçtiyse uyarı verir.
    }

    char titleToBorrow[MAX_TITLE_LENGTH];
    printf("Odunc almak istediginiz kitabin basligini girin: ");
    fgets(titleToBorrow, MAX_TITLE_LENGTH, stdin);
    titleToBorrow[strcspn(titleToBorrow, "\n")] = 0;

    int foundCount = 0;//eşleşmeyi tutacak olan değişken
    for (int i = 0; i < numBooks; i++) {
        if (strcmp(books[i].title, titleToBorrow) == 0) {
            foundCount++;
            if (books[i].status == AVAILABLE) {
                books[i].status = BORROWED;
                books[i].borrowdate = time(NULL);//sistem tarihinin ödünç alma tarihine atıyor.
                books[i].returndate = books[i].borrowdate + (15*24*60*60);
                loggedUser->borrowedBooks[loggedUser->numBorrowedBooks++] = books[i].bookId;
                printf("'%s' adli kitap (ID: %d), %s tarafindan odunc alindi.\n", books[i].title, books[i].bookId, loggedUser->name);
                printf("Iade tarihi: %s\n", formatTime(books[i].returndate));
                saveBooksToFile(BOOK_FILE, books, numBooks);
                saveUsersToFile(USER_FILE, users, numUsers);
                return;
            } else {
                printf("'%s' adli kitap (ID: %d) su anda odunc alinmis durumda.\n", books[i].title, books[i].bookId);
            }
        }
    }

    if (foundCount == 0) {
        printf("'%s' adli kitap bulunamadi.\n", titleToBorrow);
    } else {
        printf("'%s' adli tum mevcut kitaplar su anda odunc alinmis durumda.\n", titleToBorrow);
    }
}

void returnBook(User *loggedUser)
{
    if (loggedUser == NULL){
        printf("Lutfen giris yapiniz.\n");
        return;
    }

    int bookId;
    printf("Iade etmek istediginiz kitabin ID'sini giriniz: ");
    scanf("%d", &bookId);
    getchar();

    Book *bookToReturn = findBookById(bookId);//bookId parametresini alan fonksiyonla bulduğu kitabı book yapısının adresine atıyor.

    if (bookToReturn == NULL) {
        printf("Girilen ID'ye sahip kitap bulunamamistir.\n");
        return;
    }

    bool borrowedByUser = false;//kitabın ödünç alınıp alınmadığını kontrol etmek için oluşturuldu.
    int borrowedIndexInUserList = -1;//kullanıcının ödünç aldığı kitaplar listesindeki indeksi tutuyor.
    for (int i = 0; i < loggedUser->numBorrowedBooks; i++)
    {
        if (loggedUser->borrowedBooks[i] == bookId)
        {
            borrowedByUser = true;//eşleşme olduğu için true oluç
            borrowedIndexInUserList = i;//indeksi kaydedilir.
            break;
        }
    }

    if (!borrowedByUser)//eğer eşleşme olmamışsa
    {
        printf("Bu kitap size ait bir odunc kaydi bulunmuyor.\n");
        return;
    }

    time_t currentTime = time(NULL);//sistem tarihini tuta bir değişken
    if(currentTime > bookToReturn->returndate)//eğer sistem tarihi iade tarihini geçmişse
    {
        double secondsOverdue = difftime(currentTime, bookToReturn->returndate);//difftime iki tarih arasındaki farkı saniye cinsinden hesaplar.
        int daysOverdue = (int)(secondsOverdue / (24 * 60 * 60));//gün hesabı yapılır.
        printf("UYARI: Kitap %d gun gecikmeli iade edildi!\n", daysOverdue);
    }

    bookToReturn->status = AVAILABLE;//kitap durumu güncellendi.
    bookToReturn->borrowdate = 0;//ödünç alma tarihi sıfırlandı.
    bookToReturn->returndate = 0;//iade tarihi sıfırlandı.

    for (int i = borrowedIndexInUserList; i < loggedUser->numBorrowedBooks - 1; i++)//kitabın listedeki indeksinden başlayarak son indekse kadar döngü devam eder. 
    {
        loggedUser->borrowedBooks[i] = loggedUser->borrowedBooks[i + 1];//listedeki elemanları bir bir geri kaydırır.
    }
    loggedUser->numBorrowedBooks--;

    printf("'%s' adli kitap iade edildi.\n", bookToReturn->title);
    
    saveBooksToFile(BOOK_FILE, books, numBooks);
    saveUsersToFile(USER_FILE, users, numUsers);
}

void listBorrowedBooks(User *loggedUser)
{
    if (loggedUser == NULL){
        printf("Lutfen giris yapiniz.\n");
        return;
    }

    if (loggedUser->numBorrowedBooks == 0){
        printf("%s adli kullanicinin odunc aldigi herhangi bir kitap bulunmuyor.\n", loggedUser->name);
        return;
    }

    printf("\n--- %s'in Odunc Aldigi Kitaplar ---\n", loggedUser->name);

    for (int i = 0; i < loggedUser->numBorrowedBooks; i++)//kullsnıcının ödünç aldığı kitapları gezmek için oluşturuldu.
    {
        int borrowedBookId = loggedUser->borrowedBooks[i];//mevcut indeksteki(i) kitabın idsini borrowedBookId'ye atıyor.
        Book *borrowedBook = findBookById(borrowedBookId);//findBookById fonksiyonunu kullanarak kitabın bilgilerine erişiyor v işaretçi döndürü.

        if (borrowedBook != NULL){
            printf("ID: %d, Baslik: %s, Yazar: %s\n",
                   borrowedBook->bookId, borrowedBook->title, borrowedBook->author);
            if (borrowedBook->borrowdate != 0) {
                printf("  Odunc Alma Tarihi: %s\n", formatTime(borrowedBook->borrowdate));
            }
            if (borrowedBook->returndate != 0) {
                printf("  Iade Tarihi: %s\n", formatTime(borrowedBook->returndate));
            }
        }
        else{
            printf("Hata: ID %d'ye sahip kitap bulunamadi veya silindi.\n", borrowedBookId);
        }
    }
    printf("-----------------------------------\n");
}

void checkOverdueBooks(User *loggedUser)
{
    if(loggedUser == NULL){
        printf("Giris yapiniz.\n");
        return;
    }

    if(loggedUser->numBorrowedBooks == 0) {//kullanıcının ödünç aldığı kitap olup olmadığını kontrol etti.
        printf("Odunc aldiginiz kitap bulunmamaktadir.\n");
        return;
    }

    time_t currentTime = time(NULL);
    bool foundOverdue = false;
    printf("\n-----Iade Suresi Gecmis Kitaplar-----\n");
    for(int i = 0; i < loggedUser->numBorrowedBooks; i++)
    {
        int borrowedBookId = loggedUser->borrowedBooks[i];
        Book *borrowedBook = findBookById(borrowedBookId);
        
        if (borrowedBook != NULL && borrowedBook->status == BORROWED) 
        {
            if (currentTime > borrowedBook->returndate) //şu anki sistem tarihi iade tarihini geçmişse
            {
                double secondsOverdue = difftime(currentTime, borrowedBook->returndate); 
                int daysOverdue = (int)(secondsOverdue / (24 * 60 * 60));
                printf("ID: %d, Baslik: %s, Iade Tarihi: %s, Gecikme: %d gün\n",
                       borrowedBook->bookId, borrowedBook->title,
                       formatTime(borrowedBook->returndate), daysOverdue); 
                foundOverdue = true;
            }
        }
    }

    if(!foundOverdue) //foundOverdur false ise
    {
        printf("Odunc tarihi gecmis bir kitap bulunmamaktadir.\n");
    }
    printf("-------------------------------------------\n");
}

void displayAdminMenu(User *currentUser)
{
    int choice;

    do
    {
        printf("\n-----Yonetici Menusu-----\n"); 
        printf("Giris yapmis kullanici: %s (Yonetici)\n", currentUser->name); 
        printf("1. Kitap Ekle\n");
        printf("2. Kitap Guncelle\n");
        printf("3. Yeni Kullanici Ekle\n");
        printf("4. Tum Kitaplari Listele\n");
        printf("5. Basliga Gore Kitap Ara\n");
        printf("6. Kategoriye Gore Kitap Ara\n");
        printf("7. Tum Kullanicilari Listele\n");
        printf("0. Cikis Yap (Ana Menuye Don)\n");
        printf("Seciminizi yapin: ");
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1: addBook(); break;
        case 2: manageBook();break;
        case 3: addUser(); break; 
        case 4: listAllofBooks(); break;
        case 5: searchBookwithTitle(); break;
        case 6: searchBookByCategory(); break;
        case 7: listAllusers(); break;
        case 0: saveBooksToFile(BOOK_FILE, books, numBooks); 
                saveUsersToFile(USER_FILE, users, numUsers);printf("Yonetici oturumundan cikiliyor...\n"); break;
        default: printf("Hatali secim. Lutfen tekrar deneyin!\n"); break;
        }
    } while (choice != 0);
}

void displayUserMenu(User *currentUser)
{
    int choice;

    do
    {
        printf("\n-----Kullanici Menusu-----\n");
        printf("Giris yapmis kullanici: %s (Kullanici)\n", currentUser->name);
        printf("1. Tum Kitaplari Listele\n");
        printf("2. Basliga Gore Kitap Ara\n");
        printf("3. Kategoriye Gore Kitap Ara\n");
        printf("4. Kitap Odunc Al (ID ile)\n");
        printf("5. Kitap Odunc Al (Basliga Gore)\n");
        printf("6. Kitap Iade Et\n");
        printf("7. Odunc Alinan Kitaplari Listele\n");
        printf("8. Iade Suresi Dolan Kitaplari Kontrol Et\n");
        printf("0. Cikis Yap (Ana Menuye Don)\n");
        printf("Seciminizi yapin: ");
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1: listAllofBooks(); break;
        case 2: searchBookwithTitle(); break;
        case 3: searchBookByCategory(); break;
        case 4: borrowBook(currentUser); break;
        case 5: borrowBookByTitle(currentUser); break;
        case 6: returnBook(currentUser); break;
        case 7: listBorrowedBooks(currentUser); break;
        case 8: checkOverdueBooks(currentUser); break;
        case 0: saveBooksToFile(BOOK_FILE, books, numBooks);
                saveUsersToFile(USER_FILE, users, numUsers);
                printf("Kullanici oturumundan cikiliyor...\n"); break;
        default:printf("Hatali secim.Tekrar deneyin!\n"); break;
        }
    } while (choice != 0);
}

void displayLoginMenu()
{
    printf("\n-----Kutuphane Yonetim Sistemi-----\n");
    printf("1. Giris Yapiniz\n");
    printf("0. Cikis Yapiniz\n");
    printf("Seciminizi Yapiniz: ");
}


int main()
{
    numBooks = loadBooksFromFile(BOOK_FILE, books);
    numUsers = loadUsersFromFile(USER_FILE, users);

    // Uygulama ilk kez çalıştırıldığında bir varsayılan yönetici hesabı oluştur
    if (numUsers == 0) {
        User adminUser;
        adminUser.userId = 1;
        strcpy(adminUser.name, "Yonetici Admin");
        strcpy(adminUser.username, "admin");
        strcpy(adminUser.password, "admin123");
        adminUser.role = ADMIN;
        for(int i = 0; i < MAX_BORROWED_BOOKS; i++) { // Ödünç alınan kitap dizisini başlat
            adminUser.borrowedBooks[i] = 0;
        }
        users[numUsers++] = adminUser;
        printf("\nVarsayilan yonetici hesabi olusturuldu: Kullanici Adi: admin, Parola: admin123\n");
    }

    int choice;
    User *currentUser = NULL;

    do {
        if (currentUser == NULL){
            displayLoginMenu();
            scanf("%d", &choice);
            getchar();

            if (choice == 1) {
                currentUser = userAuthentication();
                if (currentUser != NULL)
                {
                    if (currentUser->role == ADMIN) {
                        displayAdminMenu(currentUser);
                    }
                    else {
                        displayUserMenu(currentUser);
                    }
                    currentUser = NULL; // Oturumdan çıktıktan sonra currentUser'ı sıfırla
                }
            }
            else if (choice == 0) {
                printf("Sistemden cikis yapiliyor...\n");
            }
            else {
                printf("Hatali secim.Tekrar deneyin.\n");
            }
        }
    } while (choice != 0);

    saveBooksToFile(BOOK_FILE, books, numBooks); 
    saveUsersToFile(USER_FILE, users, numUsers); 
    return 0;
}


