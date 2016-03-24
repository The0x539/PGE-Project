#ifndef CSVREADERQT
#define CSVREADERQT



#include "CSVReader.h"

#include "pge_file_lib_globs.h"

namespace CSVReader {
    struct CSVPGEReader
    {
        typedef PGESTRING string_type;
        CSVPGEReader(PGE_FileFormats_misc::TextInput* reader) : _reader(reader) {}

        PGESTRING read_line()
        {
            return _reader->readLine();
        }
    private:
        PGE_FileFormats_misc::TextInput* _reader;
    };

#ifdef PGE_FILES_QT
    struct CSVPGESTRINGUtils
    {
        static bool find(const QString& str, QChar sep, size_t& findIndex)
        {
            int pos = str.indexOf(sep, findIndex);
            if (pos == -1)
                return false;
            findIndex = static_cast<size_t>(pos);
            return true;
        }

        static size_t length(const QString& str)
        {
            return static_cast<size_t>(str.length());
        }

        static QString substring(const QString& str, size_t pos, size_t count)
        {
            return str.mid(pos, count);
        }
    };
#else
    struct CSVPGESTRINGUtils : DefaultStringWrapper<char, std::char_traits<char>, std::allocator<char>> {};
#endif

    #ifdef PGE_FILES_QT
    struct CSVPGESTRINGConverter
    {
        template<typename T>
        static void Convert(T* out, const QString& field)
        {
            ConvertInternal(out, field, identity<T>());
        }

    private:
        template<typename T>
        static void ConvertInternal(T* out, const QString& field, identity<T>)
        {
            (void)out;
            (void)field;
            // This is a hackishy solution, but gcc checks static_assert, even if the template function is not actually used
            static_assert(std::is_same<T, double>::value, "No default converter for this type");
        }
        static void ConvertInternal(double* out, const QString& field, identity<double>)
        {
            bool ok = true;
            *out = field.toDouble(&ok);
            if(!ok)
                throw std::invalid_argument("Could not convert to double");
        }
        static void ConvertInternal(float* out, const QString& field, identity<float>)
        {
            bool ok = true;
            *out = field.toFloat(&ok);
            if(!ok)
                throw std::invalid_argument("Could not convert to float");
        }
        static void ConvertInternal(int* out, const QString& field, identity<int>)
        {
            bool ok = true;
            *out = field.toInt(&ok);
            if(!ok)
                throw std::invalid_argument("Could not convert to int");
        }
        static void ConvertInternal(long* out, const QString& field, identity<long>)
        {
            bool ok = true;
            *out = field.toLong(&ok);
            if(!ok)
                throw std::invalid_argument("Could not convert to long");
        }
        static void ConvertInternal(long long* out, const QString& field, identity<long long>)
        {
            bool ok = true;
            *out = field.toLongLong(&ok);
            if(!ok)
                throw std::invalid_argument("Could not convert to long long");
        }
        static void ConvertInternal(long double* out, const QString& field, identity<long double>)
        {
            bool ok = true;
            *out = field.toDouble(&ok);
            if(!ok)
                throw std::invalid_argument("Could not convert to long double");
        }
        static void ConvertInternal(unsigned int* out, const QString& field, identity<unsigned int>)
        {
            bool ok = true;
            *out = static_cast<unsigned int>(field.toULong(&ok));
            if(!ok)
                throw std::invalid_argument("Could not convert to unsigned int");
        }
        static void ConvertInternal(unsigned long* out, const QString& field, identity<unsigned long>)
        {
            bool ok = true;
            *out = field.toULong(&ok);
            if(!ok)
                throw std::invalid_argument("Could not convert to unsigned long");
        }
        static void ConvertInternal(unsigned long long* out, const QString& field, identity<unsigned long long>)
        {
            bool ok = true;
            *out = field.toULongLong(&ok);
            if(!ok)
                throw std::invalid_argument("Could not convert to unsigned long long");
        }
        static void ConvertInternal(bool* out, const QString& field, identity<bool>)
        {
            if(field == "0" || field == "") // FIXME: Is it correct? Or too hackish?
                *out = false;
            else if(field == "!0" || field == "1") // FIXME: Is it correct? Or too hackish?
                *out = true;
            else
                throw std::invalid_argument(std::string("Could not convert to bool (must be empty, \"0\", \"!0\" or \"1\"), got \"") + field.toStdString() + std::string("\""));
        }
        static void ConvertInternal(QString* out, const QString& field, identity<QString>)
        {
            *out = field;
        }
    };
#else
    struct CSVPGESTRINGConverter : DefaultCSVConverter<std::string> {};
#endif

    namespace detail {
        template<class Reader>
        struct CSVReaderFromPGESTRING {
            typedef typename Reader::string_type string_type;
            typedef typename string_type::value_type value_type;

            typedef CSVReader<Reader, string_type, value_type, CSVPGESTRINGUtils, CSVPGESTRINGConverter> full_type;
        };
    }

    template<class Reader, class CharT>
    constexpr typename detail::CSVReaderFromPGESTRING<Reader>::full_type MakeCSVReaderForPGESTRING(Reader* reader, CharT sep)
    {
        return typename detail::CSVReaderFromPGESTRING<Reader>::full_type(reader, sep);
    }

}



#endif // CSVREADERQT
