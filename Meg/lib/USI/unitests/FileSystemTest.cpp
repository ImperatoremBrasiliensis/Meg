#include <gtest/gtest.h>

#include <USI/filesystem.hpp>

#include <filesystem>
#include <fstream>
#include <string>

using namespace Meg;

constexpr inline auto MEG_ALIGN(auto n, auto value) {
   return (n + value - 1) & ~(value - 1);
}

TEST(FilesystemPathIterator, ConstructorUniqueProperty) {
   filesystem::path p{"/path/to/anywhere"};

   filesystem::path::iterator i1{&p, 2};
   EXPECT_STREQ((*i1).get_string(), "ath/");

   /*
    * `pos` is too large the iterator will point 
    * to the beginning.
    */
   filesystem::path::iterator i2{&p, std::numeric_limits<size_t>::max()};
   EXPECT_STREQ((*i2).get_string(), "/");
}

TEST(FilesystemPathIterator, OperatorDereferenceUniqueProperty) {
   filesystem::path p{"/path/to/anywhere"};

   // An entire entry ('to/').
   // From current position to the separator.
   filesystem::path::iterator i1{&p, 6};
   EXPECT_STREQ((*i1).get_string(), "to/");

   // A parcial entry ('anywhere' -> 'where').
   // From current pos to the end of the path.
   filesystem::path::iterator i2{&p, 12};
   EXPECT_STREQ((*i2).get_string(), "where");
}

TEST(FilesystemPathIterator, OperatorIncrementUniqueProperty) {
   filesystem::path p{"/path/to/anywhere"};
   filesystem::path::iterator i{&p, 2};
   EXPECT_STREQ((*i).get_string(), "ath/");  // First entry (parcial, ath/).

   // Second entry ('to/').
   ++i;
   EXPECT_STREQ((*i).get_string(), "to/");

   // Last entry ('anywhere').
   ++i;
   EXPECT_STREQ((*i).get_string(), "anywhere");

   /*
    * Incrementing one more time causes 
    * the iterator to point to `end()` of
    * the path. Dereferencing `end()` is 
    * undefined behaviour.
    */
}

TEST(FilesystemPathIterator, OperatorEqualityUniqueProperty) {
   filesystem::path p1{"/path/to/anywhere"};

   filesystem::path::iterator i1{&p1, 4};
   filesystem::path::iterator i2{&p1, 4};
   filesystem::path::iterator i3{&p1, 5};

   EXPECT_TRUE(i1.operator==(i2));
   EXPECT_TRUE(i2.operator==(i1));

   EXPECT_FALSE(i1.operator==(i3));
   EXPECT_FALSE(i3.operator==(i1));

   EXPECT_FALSE(i2.operator==(i3));
   EXPECT_FALSE(i3.operator==(i2));

   /* EXTRA: iterator of diferent paths. */
   filesystem::path p2{"somewhere"};
   filesystem::path p3{"path/to/somewhere"};

   filesystem::path::iterator i4{&p2, 4};
   filesystem::path::iterator i5{&p3, 4};
   filesystem::path::iterator i6{&p2, 5};

   EXPECT_FALSE(i4.operator==(i5));
   EXPECT_FALSE(i5.operator==(i4));

   EXPECT_FALSE(i4.operator==(i6));
   EXPECT_FALSE(i6.operator==(i4));

   EXPECT_FALSE(i5.operator==(i6));
   EXPECT_FALSE(i6.operator==(i5));
}

TEST(FilesystemPath, DefaultConstructorUniqueProperty) {
   /*
	 * The Property Unique says that the `path` object will be zero-
	 * initialized.
	 */

   filesystem::path p;
   EXPECT_EQ(p.get_path_capacity(), 0);
   EXPECT_TRUE(p.is_empty());
   EXPECT_FALSE(p.get_string());
}

TEST(FilesystemPath, ConstructorFromStdStringUniqueProperty) {
   /*
	 * The Unique Property of this constructor says that this constructor
	 * is an implicit call to the `path(const char* str)` constructor, so
	 * there's no much we can test. 
	 */

   // Uses the pipe ('|') to avoid normalization.
   const char *str = "path|to|anywhere";

   filesystem::path p{std::string(str)};
   EXPECT_EQ(p.get_path_size(), 16);
   ASSERT_TRUE(p.get_string()) << "`p` may not be null!";
   EXPECT_STREQ(p.get_string(), str);
}

TEST(FilesystemPath, ConstructorFromCStringPropertyI) {
   /*
	 * Property 1 says that if `str` parameter is a valid C-style string
	 * the `path` object will be initialized with it.
	 */

   // Uses the pipe (|) to avoid separator normalization.
   const char *str = "path|to|anywhere";

   filesystem::path p{str};
   EXPECT_EQ(p.get_path_size(), 16);
   ASSERT_TRUE(p.get_string()) << "`p` may not be null!";
   EXPECT_STREQ(p.get_string(), str);
}

TEST(FilesystemPath, ConstructorFromCStringPropertyII) {
   /*
	 * Property 2nd says that `MEG_ALT_PATH_SEP` will be turned
	 * into `MEG_PATH_SEP`, consecutive path separators will be 
	 * removed and the first `MEG_VIRTUAL_DELEGATE` that is not
	 * at the beginning of the path will be turned into a null-
	 * terminator.
	 */

   static_assert(
      MEG_VIRTUAL_DELEGATE == '?',
      "Actually, the virtual delegate charater is not '?', fix this test!"
   );

   const char input[] = "?path///to\\\\some?where";
#ifdef _WIN32
   const char output[] = "?path\\to\\some";
#else
   const char output[] = "?path/to/some";
#endif

   filesystem::path p{input};
   EXPECT_EQ(p.get_path_size(), sizeof(output) - 1);
   ASSERT_TRUE(p.get_string()) << "Must be non-null!";
   EXPECT_STREQ(p.get_string(), output);
}

TEST(FilesystemPath, ConstructorFromCStringPropertyIII) {
   /*
	 * Property 4th says that passing empty strings or `nullptr` to this
	 * constructor will zero-initialize the `path` object.
	 */

   filesystem::path p1{""};
   EXPECT_EQ(p1.get_path_capacity(), 0);
   EXPECT_TRUE(p1.is_empty());
   EXPECT_FALSE(p1.get_string());

   filesystem::path p2{nullptr};
   EXPECT_EQ(p2.get_path_capacity(), 0);
   EXPECT_TRUE(p2.is_empty());
   EXPECT_FALSE(p2.get_string());
}

TEST(FilesystemPath, CopyConstructorPropertyI) {
   filesystem::path p1{"path/to/anywhere"};

   /*
	 * As the `path` parameter (`p1` in this case) is not empty,
	 * `p1` will be copied to `p2`.
	 */
   filesystem::path p2{p1};
   EXPECT_EQ(p1.get_path_capacity(), p2.get_path_capacity());
   EXPECT_EQ(p1.get_path_size(), p2.get_path_size());
   EXPECT_EQ(p1, p2);  // Compares the path (buffer/string).
   EXPECT_STREQ(p1.get_string(), "path/to/anywhere");
   EXPECT_STREQ(p2.get_string(), "path/to/anywhere");

   /* EXTRA: checks for no aliasing. */
   p1 = "";
   EXPECT_NE(p2, p1);
   EXPECT_STREQ(p2.get_string(), "path/to/anywhere");
}

TEST(FilesystemPath, CopyConstructorPropertyII) {
   filesystem::path p1{};

   /*
	 * As the `path` parameter (`p1` in this case) is empty, 
	 * `p2` should be zero-initialized.
	 */
   filesystem::path p2{p1};
   EXPECT_EQ(p2.get_path_capacity(), 0);
   EXPECT_TRUE(p2.is_empty());
   EXPECT_FALSE(p2.get_string());
}

TEST(FilesystemPath, MoveConstructorUniqueProperty) {
   // Uses the pipe (|) to avoid separator normalization.
   const char *str = "path|to|somewhere";
   filesystem::path p1{str};

   ASSERT_STREQ(p1.get_string(), str)
      << "`p1` was not correctly initialized with `str` (" << str << ")";

   const char *ptr = p1.get_string();

   /*
	 * The Unique Property says that the data property of `path` parameter will
	 * be moved into the `path` object, while the origin object will be left 
	 * with all fields as zero.
	 */
   filesystem::path p2{std::move(p1)};

   EXPECT_EQ(p1.get_path_capacity(), 0);
   EXPECT_TRUE(p1.is_empty());
   EXPECT_FALSE(p1.get_string());

   EXPECT_EQ(p2.get_path_size(), strlen(str));
   ASSERT_EQ(p2.get_string(), ptr);
   EXPECT_STREQ(p2.get_string(), str);
}

TEST(FilesystemPath, DestructorUniqueProperty) {
   /* Only to formalize. */
}

TEST(FilesystemPath, MethodGetFilenamePropertyI) {
   filesystem::path p1{"path/to/any/file.ext"};

   /*
	 * Property 1 says that if the path has a valid filename it will
	 * be returned.
	 */
   filesystem::path p2{p1.get_filename()};

   ASSERT_TRUE(p2.is_file());
   EXPECT_EQ(p2.get_path_size(), 8);  // Size of "file.ext"
   ASSERT_TRUE(p2.get_string());
   EXPECT_STREQ(p2.get_string(), "file.ext");
}

TEST(FilesystemPath, MethodGetFilenamePropertyII) {
   /*
	 * Property 2 says that if the path is empty or is `nullptr` returns
	 * a zero-initialized `path` object.
	 */
   /* Round 1 */
   filesystem::path p1{"/path/to/file"};  // Has no file extension.
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 13) << "`p1` is not empty, it should be 13";

   filesystem::path p2{p1.get_file_extension()};
   EXPECT_TRUE(p2.is_empty());

   /* Round 2 */
   filesystem::path p3{"/path/to/"};  // Has no filename.
   ASSERT_TRUE(p3.get_string()) << "`p3` may not be null";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p3` must have any memory reserve";
   ASSERT_EQ(p3.get_path_size(), 9) << "`p3` is not empty, it should be 9";

   filesystem::path p4{p3.get_file_extension()};
   EXPECT_TRUE(p4.is_empty());

   /* Round 3 */
   filesystem::path p5{};  // Empty path.
   ASSERT_TRUE(p5.is_empty()) << "`p5` must be empty";

   filesystem::path p6{p5.get_file_extension()};
   EXPECT_TRUE(p6.is_empty());

   /* Round 4 */
   filesystem::path p7{};
   ASSERT_TRUE(p7.is_empty()) << "`p7` must be empty";
   ASSERT_TRUE(p7.reserve(32)) << "Hummm... Something went wrong";
   ASSERT_TRUE(p7.is_empty()) << "`Hummm... Something went wrong";
   ASSERT_NE(p7.get_path_capacity(), 0) << "`p7 must have any memory reserve";

   filesystem::path p8{p7.get_file_extension()};
   EXPECT_TRUE(p8.is_empty());
}

TEST(FilesystemPath, MethodGetFilenamePropertyIII) {
   filesystem::path p1{"path/to/somewhere/."};

   /*
	 * Property 3 says that if the last entry is equal to "." or ".." (without path
	 * separator at the end), that is, special entry, returns a zero-initialized
	 * object.
	 */
   filesystem::path p2{p1.get_filename()};
   ASSERT_FALSE(p1.is_file()) << "'.' must not be/have a filename";
   EXPECT_EQ(p2.get_path_capacity(), 0);
   EXPECT_TRUE(p2.is_empty());
   EXPECT_FALSE(p2.get_string());

   filesystem::path p3{"path/to/somewhere/.."};

   filesystem::path p4{p3.get_filename()};
   ASSERT_FALSE(p4.is_file()) << "'..' must not be a filename";
   EXPECT_TRUE(p4.is_empty());
   EXPECT_FALSE(p4.get_string());
}

TEST(FilesystemPath, MethodGetFileExtensionPropertyI) {
   filesystem::path p1{"path/to/somewhere/file.ext"};

   /*
	 * Property 1 says that if the path has a valid filename with 
	 * a valid extension returns the file extension with dot.
	 */
   filesystem::path p2{p1.get_file_extension()};
   EXPECT_EQ(p2.get_path_size(), 4);
   ASSERT_TRUE(p2.get_string()) << "Can't continue with the test";
   EXPECT_STREQ(p2.get_string(), ".ext");
}

TEST(FilesystemPath, MethodGetFileExtensionPropertyII) {
   /*
	 * Property 2 says that if the path has no filename or file extension
	 * returns a zero-initialized `path` object. If the path has a
	 * dot (as long as it's not the first character) but without the
	 * extension name (after dot) returns only the dot.
	 */
   filesystem::path p1{"path/to/somewhere/file"};
   filesystem::path p2{p1.get_file_extension()};

   EXPECT_EQ(p2.get_path_capacity(), 0);
   EXPECT_TRUE(p2.is_empty());
   EXPECT_FALSE(p2.get_string());

   filesystem::path p3{"path/to/somewhere/file."};
   filesystem::path p4{p3.get_file_extension()};
   EXPECT_EQ(p4.get_path_size(), 1);
   ASSERT_TRUE(p4.get_string());
   EXPECT_STREQ(p4.get_string(), ".");
}

TEST(FilesystemPath, MethodGetFileExtensionPropertyIII) {
   /*
	 * Property 3 says that the dot of the file extension may not
	 * be the first character of the path. So hidden file
	 * names may not be called as file extension.
	 */

   // Valid hidden file extension.
   filesystem::path p1{"path/to/somewhere/.hidenfile.ext"};
   filesystem::path p2{p1.get_file_extension()};
   EXPECT_EQ(p2.get_path_size(), 4);
   ASSERT_TRUE(p2.get_string());
   EXPECT_STREQ(p2.get_string(), ".ext");

   // Has no file extension, .hiddenfile is a name.
   filesystem::path p3{"path/to/somewhere/.hidenfile"};
   filesystem::path p4{p3.get_file_extension()};
   EXPECT_EQ(p4.get_path_capacity(), 0);
   EXPECT_TRUE(p4.is_empty());
   EXPECT_FALSE(p4.get_string());
}

TEST(FilesystemPath, MethodGetFileExtensionPropertyIV) {
   /*
	 * Property 4 says that although special directory entries might
	 * be the last entry of the path without a path separator at the
	 * end they're not filenames and, consequently, don't have file extensions.
	 */
   filesystem::path p1{"path/to/somewhere/."};
   filesystem::path p2{p1.get_file_extension()};
   EXPECT_EQ(p2.get_path_capacity(), 0);
   EXPECT_TRUE(p2.is_empty());
   EXPECT_FALSE(p2.get_string());

   filesystem::path p3{"path/to/somewhere/.."};
   filesystem::path p4{p3.get_file_extension()};
   EXPECT_EQ(p4.get_path_capacity(), 0);
   EXPECT_TRUE(p4.is_empty());
   EXPECT_FALSE(p4.get_string());
}

TEST(FilesystemPath, MethodGetPureFilenamePropertyI) {
   /*
	 * Property 1 says that if a path have a valid filename with a
	 * valid file extension returns the file name without the file
	 * extension.
	 */
   filesystem::path p1{"path/to/somewhere/file.ext"};
   filesystem::path p2{p1.get_pure_file_name()};

   EXPECT_EQ(p2.get_path_size(), 4);
   ASSERT_TRUE(p2.get_string());
   EXPECT_STREQ(p2.get_string(), "file");

   // It's also expected.
   filesystem::path p3{"path/to/somewhere/file."};
   filesystem::path p4{p3.get_pure_file_name()};

   EXPECT_EQ(p4.get_path_size(), 4);
   ASSERT_TRUE(p4.get_string());
   EXPECT_STREQ(p4.get_string(), "file");
}

TEST(FilesystemPath, MethodGetPureFilenamePropertyII) {
   filesystem::path p1{"path/to/somewhere/"};
   filesystem::path p2{p1.get_pure_file_name()};

   EXPECT_EQ(p2.get_path_capacity(), 0);
   EXPECT_TRUE(p2.is_empty());
   EXPECT_FALSE(p2.get_string());
}

TEST(FilesystemPath, MethodGetPureFilenamePropertyIII) {
   /*
	 * Property 3 says that	if the path has a dot as the first character
	 * that dot and all after it may not be the extension.
	 */
   filesystem::path p1{"path/to/somewhere/.hiddenfile"};
   filesystem::path p2{p1.get_pure_file_name()};

   EXPECT_EQ(p2.get_path_size(), 11);
   ASSERT_TRUE(p2.get_string());
   EXPECT_STREQ(p2.get_string(), ".hiddenfile");

   filesystem::path p3{"path/to/somewhere/.hiddenfile.ext"};
   filesystem::path p4{p3.get_pure_file_name()};

   EXPECT_EQ(p4.get_path_size(), 11);
   ASSERT_TRUE(p4.get_string());
   EXPECT_STREQ(p4.get_string(), ".hiddenfile");
}

TEST(FilesystemPath, MethodGetDirectoryNamePropertyI) {
   /*
	 * Property 1 says that directory entry is any entry ended by
	 * a path separator and this method must return the last of them.
	 */
   filesystem::path p1{"path/to/somewhere/file"};
   filesystem::path p2{p1.get_directory_name()};

   EXPECT_EQ(p2.get_path_size(), 10);
   ASSERT_TRUE(p2.get_string());
   EXPECT_STREQ(p2.get_string(), "somewhere/");

   filesystem::path p3{"path/to/somewhere/"};
   filesystem::path p4{p3.get_directory_name()};

   EXPECT_EQ(p4.get_path_size(), 10);
   ASSERT_TRUE(p4.get_string());
   EXPECT_STREQ(p4.get_string(), "somewhere/");
}

TEST(FilesystemPath, MethodGetDirectoryNamePropertyII) {
   filesystem::path p1{"some.file.ext"};
   filesystem::path p2{p1.get_directory_name()};

   EXPECT_EQ(p2.get_path_capacity(), 0);
   EXPECT_TRUE(p2.is_empty());
   EXPECT_FALSE(p2.get_string());
}

TEST(FilesystemPath, MethodGetDirectoryNamePropertyIII) {
   /*
	 * Property 2 says that special directory entry may be
	 * returned but without the path separator (if there's one).
	 */

   // With no path separator at the end.
   filesystem::path p1{"path/to/somewhere/.."};
   filesystem::path p2{p1.get_directory_name()};

   EXPECT_EQ(p2.get_path_size(), 2);
   ASSERT_TRUE(p2.get_string()) << "`p2` must not be null!";
   EXPECT_STREQ(p2.get_string(), "..");

   // With path separator at the end.
   filesystem::path p3{"path/to/somewhere/../"};
   filesystem::path p4{p3.get_directory_name()};

   EXPECT_EQ(p4.get_path_size(), 3);
   ASSERT_TRUE(p4.get_string());
   EXPECT_STREQ(p4.get_string(), "../");
}

TEST(FilesystemPath, MethodGetParentPathPropertyI) {
   /*
	 * Property 1 says that if the path is valid and has more
	 * than one entry returns the path without the last entry.
	 */

   // Ending with a filename.
   filesystem::path p1{"/path/to/somewhere/file.ext"};
   filesystem::path p2{p1.get_parent_path()};

   EXPECT_EQ(p2.get_path_size(), 19);
   ASSERT_TRUE(p2.get_string());
   EXPECT_STREQ(p2.get_string(), "/path/to/somewhere/");

   // Ending with a directory.
   filesystem::path p3{"/path/to/somewhere/"};
   filesystem::path p4{p3.get_parent_path()};

   EXPECT_EQ(p4.get_path_size(), 9);
   ASSERT_TRUE(p4.get_string());
   EXPECT_STREQ(p4.get_string(), "/path/to/");
}

TEST(FilesystemPath, MethodGetParentPathPropertyII) {
   /*
	 * Property 2 says that if the path has one entry
	 * or 0 returns a zero-initialized path.
	 */

   // Here the path has only one entry.
   filesystem::path p1{"/"};
   filesystem::path p2{p1.get_parent_path()};

   EXPECT_EQ(p2.get_path_capacity(), 0);
   EXPECT_TRUE(p2.is_empty());
   EXPECT_FALSE(p2.get_string());

   // Here the path has no entries.
   filesystem::path p3{""};
   filesystem::path p4{p3.get_parent_path()};

   EXPECT_EQ(p4.get_path_capacity(), 0);
   EXPECT_TRUE(p4.is_empty());
   EXPECT_FALSE(p4.get_string());
}

TEST(FilesystemPath, MethodGetNormalizedPathPropertyI) {
   /*
	 * Property 1 says that the path must be normalized if it's
	 * not empty. The normalization stream is: 
	 * 	1 If finds a '.' special directory entry, removes it
	 *		  if there's one entry left in the path; 
	 *		2 If finds a '..' special directory entry removes it 
	 *		  and the entry before it if there's one entry left in
	 *		  the path;
	 *		3 Turns all `MEG_ALT_PATH_SEP` found into a `MEG_PATH_SEP`
	 *		  and any `MEG_VIRTUAL_DELEGATE` character that is not
	 *		  the first character of the path into a null-terminator
	 *		  character; and
	 *		4 Removes consecutive path separators.
	 */
   filesystem::path p1{"?/path\\to/elsewhere/.././folder/?anything"};
   filesystem::path p2{p1.get_normalized_path()};

   EXPECT_EQ(p2.get_path_size(), 17);
   ASSERT_TRUE(p2.get_string());
   EXPECT_STREQ(p2.get_string(), "?/path/to/folder/");

   // Has only one normal entry that can't be removed, which is '/'.
   filesystem::path p3{"?/../../."};
   filesystem::path p4{p3.get_normalized_path()};

   EXPECT_EQ(p4.get_path_size(), 8);
   ASSERT_TRUE(p4.get_string());
   EXPECT_STREQ(p4.get_string(), "?/../../");

   /*
	 * The second entry will be removed and 
	 * the last not, because the first that
	 * is a special can't be removed too
	 * (there's no entry before it).
	 */
   filesystem::path p5{"?././.."};
   filesystem::path p6{p5.get_normalized_path()};

   EXPECT_EQ(p6.get_path_size(), 5);
   ASSERT_TRUE(p6.get_string());
   EXPECT_STREQ(p6.get_string(), "?./..");
}

TEST(FilesystemPath, MethodGetNormalizedPathPropertyII) {
   // Property 2 treats of empty path.

   filesystem::path p1{""}, p2{nullptr};

   filesystem::path p3{p1.get_normalized_path()};
   EXPECT_EQ(p3.get_path_capacity(), 0);
   EXPECT_TRUE(p3.is_empty());
   EXPECT_FALSE(p3.get_string());

   filesystem::path p4{p2.get_normalized_path()};
   EXPECT_EQ(p4.get_path_capacity(), 0);
   EXPECT_TRUE(p4.is_empty());
   EXPECT_FALSE(p4.get_string());
}

TEST(FilesystemPath, MethodGetPathSizeUniqueProperty) {
   /* Round 1 */
   filesystem::path p1{"/path/to/somewhere"};
   ASSERT_TRUE(p1.get_string()) << "`p2` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";

   EXPECT_EQ(p1.get_path_size(), 18) << "`p1` is not empty, it should be 18";

   /* Round 2 */
   filesystem::path p2{};  // Empty path.
   ASSERT_TRUE(p2.is_empty()) << "`p2` must be empty";

   EXPECT_EQ(p2.get_path_size(), 0) << "`p2` is empty, it should be 0";

   /* Round 3 */
   filesystem::path p3{};
   ASSERT_TRUE(p3.is_empty()) << "`p3` must be empty";
   ASSERT_TRUE(p3.reserve(32)) << "Humm... Something went wrong";
   ASSERT_TRUE(p3.is_empty()) << "Humm... Something went wrong";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p3` must have any memory reserve";

   EXPECT_EQ(p3.get_path_size(), 0) << "`p3` is empty, it should be 0";
}

TEST(FilesystemPath, MethodGetPathCapacityUniqueProperty) {
   std::string s = std::string(128, 'A');
   filesystem::path p{s.c_str()};
   ASSERT_FALSE(p.is_empty()) << "`p` may not be empty";
   ASSERT_EQ(p.get_path_size(), 128) << "`p` is not empty, it must be 128";

   /* `reserve()` is called with how much bytes we
    * we need, but it rounds up the it to the next
    * value multiple of `filesystem::path::CAPACITY_UNITY_SIZE`.
    */
   EXPECT_EQ(
      p.get_path_capacity(),
      MEG_ALIGN(128 + (1 /* null-terminator */), filesystem::path::CAPACITY_UNITY_SIZE)
   );
}

TEST(FilesystemPath, MethodGetEntryCountUniqueProperty) {
   filesystem::path p{"entry1/entry2/entry3/entry4"};

   EXPECT_EQ(p.get_entry_count(), 4);
}

TEST(FilesystemPath, MethodGetStringUniqueProperty) {
   filesystem::path p{"/path/to/somewhere"};
   const char *str = p.get_string();
   EXPECT_STREQ(str, "/path/to/somewhere");
}

TEST(FilesystemPath, MethodReservePropertyI) {
   /*
	 * Property 1 says that if the `path` object has no
	 * memory reserve, the method allocates a memory reserve
	 * in heap to the path. The number of bytes requested is
	 *	always rounded to the next value multiple of
	 * `filesystem::path::CAPACITY_UNITY_SIZE`.
	 */

   // Zero-initialization for no memory reserve.
   filesystem::path p{};
   ASSERT_FALSE(p.get_string()) << "`p` buffer must be `nullptr`";
   ASSERT_EQ(p.get_path_capacity(), 0) << "Capacity of `p` must be 0";

   p.reserve(90);
   EXPECT_TRUE(p.get_string());
   EXPECT_EQ(p.get_path_capacity(), MEG_ALIGN(90, filesystem::path::CAPACITY_UNITY_SIZE));
}

TEST(FilesystemPath, MethodReservePropertyII) {
   /*
	 * Property 2 says that if the path already has a memory
	 * reserve the buffer will be reallocated with `n` parameter
	 * as the new size. If `n` parameter is not smaller than one
	 * third of the path's capacity the reallocation will be cancelled.
	 */

   filesystem::path p{};
   p.reserve(2048);
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_EQ(p.get_path_capacity(), MEG_ALIGN(2048, filesystem::path::CAPACITY_UNITY_SIZE))
      << "Capacity of `p` must be 2048 aligned to `filesystem::path::CAPACITY_UNITY_SIZE`";

   /*
	 * As the reservation is smaller than the capacity of 'p' (2048)
	 * but is not smaller than one third of it, doesn't reallocate.
	 *
	 * Don't forget the size alignment.
	 */
   p.reserve(1000);  // 1000 is greater than one third of 2048.
   EXPECT_EQ(p.get_path_capacity(), 2048);

   /*
	 * Here the same happens but the reservation is smaller than one
	 * third of `p`'s capacity, so the reservation will proceed.
	 *
	 * Don't forget the size alignment.
	 */
   p.reserve(600);  // 600 is less than one third of 2048.
   EXPECT_LT(p.get_path_capacity(), 2048);
   EXPECT_EQ(p.get_path_capacity(), MEG_ALIGN(600, filesystem::path::CAPACITY_UNITY_SIZE));
}

TEST(FilesystemPath, MethodReservePropertyIII) {
   filesystem::path p{"/path/to/somewhere"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";

   // Checks if `p.reserve(0)` actually frees the buffer.
   p.reserve(0);
   EXPECT_EQ(p.get_path_capacity(), 0);
   EXPECT_FALSE(p.get_string());
}

TEST(FilesystemPath, MethodFitCapacityUniqueProperty) {
   filesystem::path p{"my/path"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";

   p.fit_capacity();
   EXPECT_EQ(p.get_path_capacity(), p.get_path_size() + 1);
}

TEST(FilesystemPath, MethodErasePropertyI) {
   filesystem::path p{"path/to/somewhere"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";

   EXPECT_TRUE(p.erase(4, 8)) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "path/to/where");
}

TEST(FilesystemPath, MethodErasePropertyII) {
   filesystem::path p{"path/to/somewhere"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";

   // Extraordinary size passed to `n` parameter.
   EXPECT_TRUE(p.erase(40, 7));
   EXPECT_STREQ(p.get_string(), "path/to");

   // The position 10 doesn't exists.
   EXPECT_FALSE(p.erase(1, 10));
   EXPECT_STREQ(p.get_string(), "path/to");

   // The `n` parameter is 0, the method must return `false`.
   EXPECT_FALSE(p.erase(0, 1));
   EXPECT_STREQ(p.get_string(), "path/to");
}

TEST(FilesystemPath, MethodRemoveUniqueProperty) {
   filesystem::path p{"path/to/somewhere"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";

   EXPECT_TRUE(p.remove()) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "path/to/");

   p.clear();
   ASSERT_TRUE(p.is_empty()) << "Can't continue with the test";
   EXPECT_FALSE(p.remove()) << "`p` is empty, must return `false`";
   EXPECT_TRUE(p.is_empty());
}

TEST(FilesystemPath, MethodReplaceFilenameUniqueProperty) {
   filesystem::path p{"path/to/some/folder/"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";
   ASSERT_EQ(p.get_path_size(), 20) << "`p` is not empty, it should be 20";

   // `p` has no filename, let's add a new one.
   EXPECT_TRUE(p.replace_filename("MyFile.ext")) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "path/to/some/folder/MyFile.ext");

   // `p` has a filename, let's change it.
   EXPECT_TRUE(p.replace_filename("OtherFilename.ext")) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "path/to/some/folder/OtherFilename.ext");

   // `p` has a new filename, let's remove it.
   EXPECT_TRUE(p.replace_filename("")) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "path/to/some/folder/");

   // EXTRA: remove with remove with `nullptr`.
   filesystem::path pext{"path/to/some/folder/AFile.ext"};
   ASSERT_TRUE(pext.get_string()) << "`pext` may not be null";
   ASSERT_NE(pext.get_path_capacity(), 0) << "`pext` must have any memory reserve";
   ASSERT_EQ(pext.get_path_size(), 29) << "`pext` is not empty, it should be 29";

   EXPECT_TRUE(pext.replace_filename(nullptr)) << "Humm...Something went wrong";
   EXPECT_STREQ(pext.get_string(), "path/to/some/folder/");
}

TEST(FilesystemPath, MethodReplaceFileExtensionPropertyI) {
   filesystem::path p{"somefile"};
   EXPECT_TRUE(p.get_string()) << "`p` may not be null";
   EXPECT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";

   // Adds a new extension.
   EXPECT_TRUE(p.replace_file_extension("myext")) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "somefile.myext");

   // Replaces the extension.
   EXPECT_TRUE(p.replace_file_extension("newext")) << "HUMM... Something get wrong";
   EXPECT_STREQ(p.get_string(), "somefile.newext");

   // Removes the extension.
   EXPECT_TRUE(p.replace_file_extension("")) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "somefile");
}

TEST(FilesystemPath, MethodReplaceFileExtensionPropertyII) {
   // Path withou filename.
   filesystem::path p{"path/to/folder/"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";
   ASSERT_FALSE(p.is_file()) << "`p` should not have a filename";

   EXPECT_FALSE(p.replace_file_extension("newext"));
   EXPECT_STREQ(p.get_string(), "path/to/folder/");

   // Hidden filenames are not extensions.
   ASSERT_TRUE(p.assign(".ahiddenfilename")) << "Can't continue with the test";

   EXPECT_TRUE(p.replace_file_extension("otherext")) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), ".ahiddenfilename.otherext");

   EXPECT_TRUE(p.replace_file_extension("")) << "Humm... Something went wrong";
   EXPECT_FALSE(p.replace_file_extension("")) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), ".ahiddenfilename");
}

TEST(FilesystemPath, MethodClearUniqueProperty) {
   filesystem::path p{"path/"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";
   ASSERT_EQ(p.get_path_size(), 5) << "`p` is not empty, it size should be 5";

   p.clear();
   EXPECT_TRUE(p.is_empty() && !p.get_string());
}

TEST(FilesystemPath, MethodInsertPropertyI) {
   /*
    * Property 1 says that if the `str` parameter 
    * is not null, it will be inserted `pos` (param.)
    * bytes from the beginning of the path.
    */

   /* Round 1: in the beginning of the path. */
   filesystem::path p1{"to/somewhere"};
   ASSERT_TRUE(p1.get_string()) << "`p2` must not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 12) << "`p2` is not empty, it should be 12";

   EXPECT_TRUE(p1.insert("path/", 0));

   EXPECT_EQ(p1.get_path_size(), 17);
   ASSERT_TRUE(p1.get_string()) << "Humm... Something went wrong";
   EXPECT_STREQ(p1.get_string(), "path/to/somewhere");

   /* Round 2: in a specific position. */
   filesystem::path p2{"path/to/somewhere"};
   ASSERT_TRUE(p2.get_string()) << "`p2` must not be null";
   ASSERT_NE(p2.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p2.get_path_size(), 17) << "`p2` is not empty, it should be 17";

   EXPECT_TRUE(p2.insert("/a/beautiful", 7));

   EXPECT_EQ(p2.get_path_size(), 29);
   ASSERT_TRUE(p2.get_string()) << "Humm... Something went wrong";
   EXPECT_STREQ(p2.get_string(), "path/to/a/beautiful/somewhere");

   /* Round extra: using a zero initialized path. */
   filesystem::path p3{};
   ASSERT_TRUE(p3.is_empty()) << "`p3` must not be null";

   EXPECT_TRUE(p3.insert("path", 0));

   EXPECT_EQ(p3.get_path_size(), 4);
   ASSERT_TRUE(p3.get_string()) << "Humm... Something went wrong";
   EXPECT_STREQ(p3.get_string(), "path");
}

TEST(FilesystemPath, MethodInsertPropertyII) {
   /*
    * Property 2 says that if `pos` parameter is greater 
    * than path size the insertion will be at the end of
    * path.
    */
   filesystem::path p{"path/to"};
   ASSERT_TRUE(p.get_string()) << "`p` must not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";
   ASSERT_EQ(p.get_path_size(), 7) << "`p` is not empty, it should be 7";

   EXPECT_TRUE(p.insert("/somewhere", std::numeric_limits<size_t>::max()));

   EXPECT_EQ(p.get_path_size(), 17);
   ASSERT_TRUE(p.get_string()) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "path/to/somewhere");
}

TEST(FilesystemPath, MethodInsertPropertyIII) {
   filesystem::path p{"path/to/somewhere"};
   ASSERT_TRUE(p.get_string()) << "`p` must not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";
   ASSERT_EQ(p.get_path_size(), 17) << "`p` is not empty, it should be 17";

   /* Round 1: nothing happens. */
   EXPECT_FALSE(p.insert(nullptr, 0));  // Inserts `nullptr`.

   EXPECT_EQ(p.get_path_size(), 17);
   ASSERT_TRUE(p.get_string()) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "path/to/somewhere");

   /* Round 2: nothing happens but returns true. */
   EXPECT_TRUE(p.insert("", 0));  // Inserts an empty string.

   EXPECT_EQ(p.get_path_size(), 17);
   ASSERT_TRUE(p.get_string()) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "path/to/somewhere");
}

TEST(FilesystemPath, MethodAssignUniqueProperty) {
   filesystem::path p{"a/path"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";
   ASSERT_EQ(p.get_path_size(), 6) << "`p` is not empty, it should be 6";

   EXPECT_TRUE(p.assign("other/path")) << "Humm... Something went wrong";
   EXPECT_STREQ(p.get_string(), "other/path");

   // Clears.
   EXPECT_TRUE(p.assign("")) << "Humm... Something went wrong";
   EXPECT_TRUE(p.is_empty() && !p.get_string());

   // EXTRA: Clears with `nullptr`.
   filesystem::path pext{"an/extra/path"};
   ASSERT_TRUE(pext.get_string()) << "`pext` may not be null";
   ASSERT_NE(pext.get_path_capacity(), 0) << "`pext` must have any memory reserve";

   EXPECT_TRUE(pext.assign(nullptr)) << "Humm... Something went wrong";
   EXPECT_TRUE(pext.is_empty() && !p.get_string());
}

TEST(FilesystemPath, MethodAppendPropertyI) {
   /*
    * Property 1 says that if `element` (param.) is
    * not `nullptr` inserts it at the end of the path.
    * Case the path ends in a file entry, this mehod
    * also inserts a path separator atthe end before 
    * insert the string.
    */

   /* Round 1: inserts an extra separator. */
   filesystem::path p1{"folder/my_file.txt"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 18) << "`p1` is not empty, it should be 18";

   EXPECT_TRUE(p1.append("my_file.png"));

   EXPECT_EQ(p1.get_path_size(), 30);
   ASSERT_TRUE(p1.get_string()) << "Humm... Something went wrong";
   EXPECT_STREQ(p1.get_string(), "folder/my_file.txt/my_file.png");

   /* Round 2: doesn't inserts an extra separator. */
   filesystem::path p2{"folder/folder2/"};
   ASSERT_TRUE(p2.get_string()) << "`p2` may not be null";
   ASSERT_NE(p2.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p2.get_path_size(), 15) << "`p2` is not empty, it should be 15";

   EXPECT_TRUE(p2.append("my_file.png"));

   EXPECT_EQ(p2.get_path_size(), 26);
   ASSERT_TRUE(p2.get_string()) << "Humm... Something went wrong";
   EXPECT_STREQ(p2.get_string(), "folder/folder2/my_file.png");
}

TEST(FilesystemPath, MethodAppendPropertyII) {
   /*
    * Property 2 says that if `element` (param.) 
    * is absulute (that is, begins with a separator),
    * the path will be replaced with it and will 
    * virtual if the parameter also has a virtual
    * delegate character.
    */

   /* Round 1: Turns the path not virtual. */
   filesystem::path p1{"?path/to/somewhere"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 18) << "`p1` is not empty, it should be 18";
   ASSERT_TRUE(p1.is_virtual()) << "`p1` must be virtual";

   // Replaces the path and virtualizes it.
   EXPECT_TRUE(p1.append("/absolute/path/to/somewhere"));

   EXPECT_EQ(p1.get_path_size(), 27);
   EXPECT_FALSE(p1.is_virtual());
   ASSERT_TRUE(p1.get_string()) << "Humm... Something went wrong";
   EXPECT_STREQ(p1.get_string(), "/absolute/path/to/somewhere");

   /* Round 2: Turns the path virtual. */
   filesystem::path p2{"path/to/somewhere"};
   ASSERT_TRUE(p2.get_string()) << "`p2` may not be null";
   ASSERT_NE(p2.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p2.get_path_size(), 17) << "`p2` is not empty, it should be 17";
   ASSERT_FALSE(p2.is_virtual()) << "`p2` must not be virtual";

   // Replaces the path and desvirtualizes it.
   EXPECT_TRUE(p2.append("?/absolute/path/to/somewhere"));

   EXPECT_EQ(p2.get_path_size(), 28);
   EXPECT_TRUE(p2.is_virtual());
   ASSERT_TRUE(p2.get_string()) << "Humm... Something went wrong";
   EXPECT_STREQ(p2.get_string(), "?/absolute/path/to/somewhere");
}

TEST(FilesystemPath, MethodNormalizeUniqueProperty) {
   char path[] = "?/path\\\\\\to/elsewhere/.././////folder/?anything";
#ifdef _WIN32
   char result[] = "?\\path\\to\\folder\\";
#else
   char result[] = "?/path/to/folder/";
#endif

   filesystem::path p{};
   ASSERT_TRUE(p.is_empty()) << "`p` must be empty";

   // Operator= don't normalizes the path.
   p = path;
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";
   ASSERT_EQ(p.get_path_size(), strlen(path)) << "`p` is not empty, it should be" << strlen(path);

   EXPECT_TRUE(p.normalize()) << "Humm... Something went wrong";

   ASSERT_TRUE(p.get_string());
   EXPECT_STREQ(p.get_string(), result);
}

TEST(FilesystemPath, MethodVirtualizeUniqueProperty) {
   filesystem::path p{"/path/to/somewhere"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";
   ASSERT_EQ(p.get_path_size(), 18) << "`p` is not empty, it should be 18";

   EXPECT_TRUE(p.virtualize(true)) << "Humm... Something went wrong";
   ASSERT_TRUE(p.get_string());
   EXPECT_STREQ(p.get_string(), "?/path/to/somewhere");

   EXPECT_TRUE(p.virtualize(true)) << "Humm... Something went wrong";
   ASSERT_TRUE(p.get_string());
   EXPECT_STREQ(p.get_string(), "/path/to/somewhere");
}

TEST(FilesystemPath, MethodComparePropertyI) {
   /* Round 1: with two equal paths. */
   const filesystem::path p1{"path/to/somewhere"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 17) << "`p1` is not empty, it should be 17";

   const filesystem::path p2{"path/to/somewhere"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 17) << "`p1` is not empty, it should be 17";

   EXPECT_STREQ(p1.get_string(), p2.get_string());
   EXPECT_TRUE(p1.compare(p2));
   EXPECT_TRUE(p2.compare(p1));

   /* Round 2: with unequal paths. */
   const filesystem::path p3{"?path/to/somewhere"};
   ASSERT_TRUE(p3.get_string()) << "`p3` may not be null";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p3` must have any memory reserve";
   ASSERT_EQ(p3.get_path_size(), 18) << "`p3` is not empty, it should be 18";

   EXPECT_STRNE(p3.get_string(), p1.get_string());
   EXPECT_FALSE(p3.compare(p1));
   EXPECT_FALSE(p1.compare(p3));
}

TEST(FilesystemPath, MethodComparePropertyII) {
   const filesystem::path p1{}, p2{};  // Two empty paths.
   ASSERT_TRUE(p1.is_empty()) << "`p1` must be empty";
   ASSERT_TRUE(p2.is_empty()) << "`p2` must be empty";

   EXPECT_TRUE(p1.compare(p2));
   EXPECT_TRUE(p2.compare(p1));

   const filesystem::path p3{"/path/to/somewhere"};
   ASSERT_TRUE(p3.get_string()) << "`p3` may not be null";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p3` must have any memory reserve";
   ASSERT_EQ(p3.get_path_size(), 18) << "`p8` is not empty, it should be 18";

   EXPECT_FALSE(p1.compare(p3));
   EXPECT_FALSE(p3.compare(p1));
}

TEST(FilesystemPath, MethodHasDirectoriesUniqueProperty) {
   filesystem::path p1{"/"};
   ASSERT_TRUE(p1.get_string()) << "`p2` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 1) << "`p2` is not empty, it should be 1";

   filesystem::path p2{"/file.ext"};
   ASSERT_TRUE(p2.get_string()) << "`p2` may not be null";
   ASSERT_NE(p2.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p2.get_path_size(), 9) << "`p2` is not empty, it should be 9";

   filesystem::path p3{"."};
   ASSERT_TRUE(p3.get_string()) << "`p3` may not be null";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p3` must have any memory reserve";
   ASSERT_EQ(p3.get_path_size(), 1) << "`p3` is not empty, it should be 1";

   filesystem::path p4{".."};
   ASSERT_TRUE(p4.get_string()) << "`p4` may not be null";
   ASSERT_NE(p4.get_path_capacity(), 0) << "`p4` must have any memory reserve";
   ASSERT_EQ(p4.get_path_size(), 2) << "`p4` is not empty, it should be 2";

   filesystem::path p5{"file.ext"};
   ASSERT_TRUE(p5.get_string()) << "`p5` may not be null";
   ASSERT_NE(p5.get_path_capacity(), 0) << "`p5` must have any memory reserve";
   ASSERT_EQ(p5.get_path_size(), 8) << "`p5` is not empty, it should be 8";

   filesystem::path p6{};
   ASSERT_TRUE(p6.is_empty()) << "`p6` must be empty";

   EXPECT_TRUE(p1.has_directories());
   EXPECT_TRUE(p2.has_directories());
   EXPECT_TRUE(p3.has_directories());
   EXPECT_TRUE(p4.has_directories());
   EXPECT_FALSE(p5.has_directories());
   EXPECT_FALSE(p6.has_directories());
}

TEST(FilesystemPath, MethodIsFileUniqueProperty) {
   filesystem::path p1{"/folder/folder/file.ext"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 23) << "`p1` is not empty, it should be 23";

   filesystem::path p2{"/folder/folder/imNotAFile.ext/"};
   ASSERT_TRUE(p2.get_string()) << "`p2` may not be null";
   ASSERT_NE(p2.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p2.get_path_size(), 30) << "`p2` is not empty, it should be 30";

   filesystem::path p3{};
   ASSERT_TRUE(p3.is_empty()) << "`p3` must be empty";

   EXPECT_TRUE(p1.is_file());
   EXPECT_FALSE(p2.is_file());
   EXPECT_FALSE(p3.is_file());
}

TEST(FilesystemPath, MethodIsAbsoluteUniqueProperty) {
   filesystem::path p1{"/folderInRoot/fileInFolderInRoot.ext"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 36) << "`p1` is not empty, it should be 36";

   filesystem::path p2{"?/"};
   ASSERT_TRUE(p2.get_string()) << "`p2` may not be null";
   ASSERT_NE(p2.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p2.get_path_size(), 2) << "`p2` is not empty, it should be 2";

   filesystem::path p3{"file.ext"};
   ASSERT_TRUE(p3.get_string()) << "`p3` may not be null";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p3` must have any memory reserve";
   ASSERT_EQ(p3.get_path_size(), 8) << "`p3` is not empty, it should be 8";

   filesystem::path p4{""};
   ASSERT_TRUE(p4.is_empty()) << "`p4` must be empty";

   EXPECT_TRUE(p1.is_absolute());
   EXPECT_TRUE(p2.is_absolute());
   EXPECT_FALSE(p3.is_absolute());
   EXPECT_FALSE(p4.is_absolute());
}

TEST(FilesystemPath, MethodIsDirectoryUniqueProperty) {
   // The last entry ends in a separator, so it's a directory entry.
   filesystem::path p1{"/path/to/somewhere/"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 19) << "`p1` is not empty, it should be 19";

   filesystem::path p2{"/"};
   ASSERT_TRUE(p2.get_string()) << "`p2` may not be null";
   ASSERT_NE(p2.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p2.get_path_size(), 1) << "`p2` is not empty, it should be 1";

   filesystem::path p3{"?/path/to/somewhere/"};
   ASSERT_TRUE(p3.get_string()) << "`p3` may not be null";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p3` must have any memory reserve";
   ASSERT_EQ(p3.get_path_size(), 20) << "`p3` is not empty, it should be 20";

   filesystem::path p4{"?/"};
   ASSERT_TRUE(p4.get_string()) << "`p4` may not be null";
   ASSERT_NE(p4.get_path_capacity(), 0) << "`p4` must have any memory reserve";
   ASSERT_EQ(p4.get_path_size(), 2) << "`p4` is not empty, it should be 2";

   filesystem::path p5{"file.ext"};
   ASSERT_TRUE(p5.get_string()) << "`p5` may not be null";
   ASSERT_NE(p5.get_path_capacity(), 0) << "`p5` must have any memory reserve";
   ASSERT_EQ(p5.get_path_size(), 8) << "`p5` is not empty, it should be 8";

   filesystem::path p6{};
   ASSERT_TRUE(p6.is_empty()) << "`p6` must be empty";

   EXPECT_TRUE(p1.is_directory());
   EXPECT_TRUE(p2.is_directory());
   EXPECT_TRUE(p3.is_directory());
   EXPECT_TRUE(p4.is_directory());
   EXPECT_FALSE(p5.is_directory());
   EXPECT_FALSE(p6.is_directory());
}

TEST(FilesystemPath, MethodIsEmptyUniqueProperty) {
   filesystem::path p1{};  // Empty.

   filesystem::path p2{};  // Must be empty and have any memory reserve.
   ASSERT_TRUE(p2.is_empty()) << "`p2` must be empty";
   ASSERT_TRUE(p2.reserve(32)) << "Humm... Something went wrong";
   ASSERT_TRUE(p2.is_empty()) << "Humm... Something went wrong";
   ASSERT_NE(p2.get_path_capacity(), 0) << "`p2` must have any memory reserve";

   filesystem::path p3{"any/path"};
   ASSERT_TRUE(p3.get_string()) << "`p3` may not be null";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p3` must have any memory reserve";
   ASSERT_EQ(p3.get_path_size(), 8) << "`p3` is not empty, it should be 8";

   EXPECT_TRUE(p1.is_empty());
   EXPECT_TRUE(p2.is_empty());
   EXPECT_FALSE(p3.is_empty());
}

TEST(FilesystemPath, MethodIsVirtualUniqueProperty) {
   filesystem::path p1{"?/path/to/somewhere"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 19) << "`p1` is not empty, it should be 19";

   filesystem::path p2{"/path/to/somewhere"};
   ASSERT_TRUE(p2.get_string()) << "`p2` may not be null";
   ASSERT_NE(p2.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p2.get_path_size(), 18) << "`p2` is not empty, it should be 18";

   filesystem::path p3{"?"};
   ASSERT_TRUE(p3.get_string()) << "`p3` may not be null";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p3` must have any memory reserve";
   ASSERT_EQ(p3.get_path_size(), 1) << "`p3` is not empty, it should be 1";

   filesystem::path p4{};  // Empty.
   ASSERT_TRUE(p4.is_empty()) << "`p4` must be empty";

   EXPECT_TRUE(p1.is_virtual());
   EXPECT_FALSE(p2.is_virtual());
   EXPECT_TRUE(p3.is_virtual());
   EXPECT_FALSE(p4.is_virtual());
}

TEST(FilesystemPath, MethodBeginUniqueProperty) {
   /* Round 1: returns a valid iterator`. */
   filesystem::path p1{"?/path/to/somewhere"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 19) << "`p1` is not empty, it should be 19";

   std::string s{};
   for (const filesystem::path &entry: p1)
      s.append(entry.get_string());

   EXPECT_STREQ(p1.get_string(), s.c_str());
   ASSERT_NE(p1.begin(), p1.end());

   /* Round 2: empty path returns `end()`. */
   filesystem::path p2{};  // It has no reserve.
   ASSERT_TRUE(p2.is_empty());
   ASSERT_EQ(p2.get_path_capacity(), 0);

   filesystem::path p3{};  // Must be empty and have any memory reserve.
   ASSERT_TRUE(p3.is_empty()) << "`p4` must be empty";
   ASSERT_TRUE(p3.reserve(32)) << "Humm... Something went wrong";
   ASSERT_TRUE(p3.is_empty()) << "Humm... Something went wrong";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p4` must have any memory reserve";

   EXPECT_EQ(p2.begin(), p2.end());
   EXPECT_EQ(p3.begin(), p3.end());
}

TEST(FilesystemPath, MethodEndUniqueProperty) {
   filesystem::path p1{"/path/to/somewhere"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 18) << "`p1` is not empty, it should be 18";

   std::string s{};
   for (const filesystem::path &entry: p1)
      s.append(entry.get_string());

   EXPECT_STREQ(p1.get_string(), s.c_str());
   EXPECT_NE(p1.end(), p1.begin());
}

TEST(FilesystemPath, OperatorBoolUniqueProperty) {
   filesystem::path p1{"myPath"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 6) << "`p1` is not empty, it should be 6";
   ASSERT_FALSE(p1.is_empty()) << "`p1` must be empty";

   filesystem::path p2{};
   ASSERT_TRUE(p2.is_empty()) << "`p2` must be empty";

   EXPECT_TRUE(p1.operator bool());
   EXPECT_FALSE(p2.operator bool());
}

TEST(FilesystemPath, OperatorAssignUniqueProperty) {
   filesystem::path p{"a/path"};
   ASSERT_TRUE(p.get_string()) << "`p` may not be null";
   ASSERT_NE(p.get_path_capacity(), 0) << "`p` must have any memory reserve";
   ASSERT_EQ(p.get_path_size(), 6) << "`p` is not empty, it should be 6";

   p.operator=("other/path");
   EXPECT_STREQ(p.get_string(), "other/path");

   // Clears.
   p.operator=("");
   EXPECT_TRUE(p.is_empty() && !p.get_string());
   EXPECT_EQ(p.get_path_capacity(), 0);

   // EXTRA: Clears with `nullptr`.
   filesystem::path pext{"an/extra/path"};
   ASSERT_TRUE(pext.get_string()) << "`pext` may not be null";
   ASSERT_NE(pext.get_path_capacity(), 0) << "`pext` must have any memory reserve";

   pext.operator=(nullptr);
   EXPECT_TRUE(pext.is_empty() && !pext.get_string());
   EXPECT_EQ(pext.get_path_capacity(), 0);
}

TEST(FilesystemPath, OperatorEqualityUniqueProperty) {
   filesystem::path p1{"/path/to/somewhere"};
   ASSERT_TRUE(p1.get_string()) << "`p1` may not be null";
   ASSERT_NE(p1.get_path_capacity(), 0) << "`p1` must have any memory reserve";
   ASSERT_EQ(p1.get_path_size(), 18) << "`p1` is not empty, it should be 18";

   filesystem::path p2{"/path/to/somewhere"};
   ASSERT_TRUE(p2.get_string()) << "`p2` may not be null";
   ASSERT_NE(p2.get_path_capacity(), 0) << "`p2` must have any memory reserve";
   ASSERT_EQ(p2.get_path_size(), 18) << "`p2` is not empty, it should be 18";

   filesystem::path p3{"?/path/to/somewhere"};
   ASSERT_TRUE(p3.get_string()) << "`p3` may not be null";
   ASSERT_NE(p3.get_path_capacity(), 0) << "`p3` must have any memory reerve";
   ASSERT_EQ(p3.get_path_size(), 19) << "`p3` is not empty, it should be 19";

   EXPECT_TRUE(p1.operator==(p2));
   EXPECT_FALSE(p1.operator==(p3));
}

int main(int argc, char *argv[]) {
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
