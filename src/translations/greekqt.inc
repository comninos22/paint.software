// Built-in fallback for Qt's standard widgets when qtbase_el.qm is unavailable.
// Native desktop dialogs may use the desktop's own language instead.
class GreekQtTranslator final : public QTranslator {
public:
    bool isEmpty() const override { return false; }
    QString translate(const char *context, const char *sourceText,
                      const char * = nullptr, int n = -1) const override {
        const QByteArray c(context);
        if (n != -1 || (c != "QPlatformTheme" && c != "QDialogButtonBox"
            && c != "QMessageBox" && c != "QFileDialog" && c != "QColorDialog"
            && c != "QLineEdit" && c != "QWidgetTextControl"))
            return {};
        static const QHash<QString, QString> strings = {
            {"OK", "Εντάξει"}, {"Save", "Αποθήκευση"},
            {"Save All", "Αποθήκευση όλων"}, {"Open", "Άνοιγμα"},
            {"&Yes", "&Ναι"}, {"Yes to &All", "Ναι σε ό&λα"},
            {"&No", "Ό&χι"}, {"N&o to All", "Όχι σε όλ&α"},
            {"Abort", "Ματαίωση"}, {"Retry", "Επανάληψη"},
            {"Ignore", "Παράβλεψη"}, {"Close", "Κλείσιμο"},
            {"Cancel", "Ακύρωση"}, {"Discard", "Απόρριψη"},
            {"Help", "Βοήθεια"}, {"Apply", "Εφαρμογή"},
            {"Reset", "Επαναφορά"}, {"Restore Defaults", "Επαναφορά προεπιλογών"},
            {"Show Details...", "Εμφάνιση λεπτομερειών..."},
            {"Hide Details...", "Απόκρυψη λεπτομερειών..."},
            {"Look in:", "Αναζήτηση σε:"}, {"Back", "Πίσω"},
            {"Go back", "Μετάβαση πίσω"}, {"Forward", "Εμπρός"},
            {"Go forward", "Μετάβαση εμπρός"},
            {"Parent Directory", "Γονικός φάκελος"},
            {"Go to the parent directory", "Μετάβαση στον γονικό φάκελο"},
            {"Create New Folder", "Δημιουργία νέου φακέλου"},
            {"Create a New Folder", "Δημιουργία νέου φακέλου"},
            {"List View", "Προβολή λίστας"},
            {"Change to list view mode", "Αλλαγή σε προβολή λίστας"},
            {"Detail View", "Προβολή λεπτομερειών"},
            {"Change to detail view mode", "Αλλαγή σε προβολή λεπτομερειών"},
            {"Sidebar", "Πλευρική στήλη"},
            {"List of places and bookmarks", "Λίστα τοποθεσιών και σελιδοδεικτών"},
            {"Files", "Αρχεία"}, {"Files of type:", "Τύπος αρχείων:"},
            {"Find Directory", "Εύρεση φακέλου"}, {"Save As", "Αποθήκευση ως"},
            {"Directory:", "Φάκελος:"}, {"File &name:", "Ό&νομα αρχείου:"},
            {"&Open", "Ά&νοιγμα"}, {"&Choose", "&Επιλογή"},
            {"&Save", "Απο&θήκευση"}, {"All Files (*)", "Όλα τα αρχεία (*)"},
            {"All files (*)", "Όλα τα αρχεία (*)"}, {"Show ", "Εμφάνιση "},
            {"&Rename", "&Μετονομασία"}, {"&Delete", "&Διαγραφή"},
            {"Show &hidden files", "Εμφάνιση &κρυφών αρχείων"},
            {"&New Folder", "&Νέος φάκελος"}, {"Directories", "Φάκελοι"},
            {"%1\nDirectory not found.\nPlease verify the correct directory name was given.",
             "%1\nΟ φάκελος δεν βρέθηκε.\nΕλέγξτε ότι το όνομα του φακέλου είναι σωστό."},
            {"%1 already exists.\nDo you want to replace it?",
             "Το %1 υπάρχει ήδη.\nΘέλετε να το αντικαταστήσετε;"},
            {"%1\nFile not found.\nPlease verify the correct file name was given.",
             "%1\nΤο αρχείο δεν βρέθηκε.\nΕλέγξτε ότι το όνομα του αρχείου είναι σωστό."},
            {"New Folder", "Νέος φάκελος"}, {"Delete", "Διαγραφή"},
            {"'%1' is write protected.\nDo you want to delete it anyway?",
             "Το '%1' προστατεύεται από εγγραφή.\nΘέλετε να το διαγράψετε ούτως ή άλλως;"},
            {"Are you sure you want to delete '%1'?", "Θέλετε σίγουρα να διαγράψετε το '%1';"},
            {"Could not delete directory.", "Αδυναμία διαγραφής του φακέλου."},
            {"Recent Places", "Πρόσφατες τοποθεσίες"}, {"Remove", "Αφαίρεση"},
            {"My Computer", "Ο υπολογιστής μου"}, {"Drive", "Μονάδα δίσκου"},
            {"%1 File", "Αρχείο %1"}, {"File", "Αρχείο"},
            {"File Folder", "Φάκελος αρχείων"}, {"Folder", "Φάκελος"},
            {"Alias", "Ψευδώνυμο"}, {"Shortcut", "Συντόμευση"}, {"Unknown", "Άγνωστο"},
            {"Hu&e:", "Από&χρωση:"}, {"&Sat:", "&Κορεσμός:"},
            {"&Val:", "&Τιμή:"}, {"&Red:", "Κό&κκινο:"},
            {"&Green:", "&Πράσινο:"}, {"Bl&ue:", "Μ&πλε:"},
            {"A&lpha channel:", "Κανάλι ά&λφα:"}, {"&HTML:", "&HTML:"},
            {"Cursor at %1, %2\nPress ESC to cancel", "Δείκτης στο %1, %2\nΠατήστε ESC για ακύρωση"},
            {"Select Color", "Επιλογή χρώματος"}, {"&Basic colors", "&Βασικά χρώματα"},
            {"&Custom colors", "Π&ροσαρμοσμένα χρώματα"},
            {"&Add to Custom Colors", "Προ&σθήκη στα προσαρμοσμένα χρώματα"},
            {"&Pick Screen Color", "Επι&λογή χρώματος οθόνης"},
            {"&Undo", "&Αναίρεση"}, {"&Redo", "Α&κύρωση αναίρεσης"},
            {"Cu&t", "Απο&κοπή"}, {"&Copy", "Αντι&γραφή"},
            {"&Paste", "Επικό&λληση"}, {"Select All", "Επιλογή όλων"},
            {"Select &All", "Επιλογή ό&λων"},
        };
        return strings.value(QString::fromUtf8(sourceText));
    }
};
