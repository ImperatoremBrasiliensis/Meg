#include <gtest/gtest.h>

#include <USI/filesystem.hpp>

#include <filesystem>
#include <fstream>

using namespace Meg;

class ReaderTest: public testing::Test {
protected:
   std::string filename = "test_file.txt";
   filesystem::path temp_file_path =
      std::filesystem::temp_directory_path()
         .append(filename)
         .c_str();

   void TearDown() override {
      if (std::filesystem::exists(temp_file_path.get_string())) {
         std::filesystem::remove(temp_file_path.get_string());
      }
   }
};

TEST_F(ReaderTest, ConstructorUniqueProperty) {
   std::string buf, content{"Hello from the Meg Project."};
   std::ofstream out{temp_file_path.get_string()};
   out << content;
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value())
      << "Unable to get the temporary test file. Err code: " << (int) f.error();

   filesystem::reader r{f.value()};
   ASSERT_TRUE(r.is_loaded()) << "File isn't loaded.";

   buf.resize(content.size());
   r.read(buf.data(), buf.size());

   EXPECT_EQ(buf, content);
}

TEST_F(ReaderTest, MethodReadPropertyI) {
   std::string content{"Hello from the Meg Project!!!"};
   std::ofstream out{temp_file_path.get_string()};
   out << content;
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value())
      << "Unable to get the temporary test file. Err code: " << (int) f.error();

   filesystem::reader r{f.value()};
   ASSERT_TRUE(r.is_loaded()) << "File isn't loaded";

   unsigned offset = 0;

   /* Round 1 */ {
      // Reads 'Hello ', 6 chars.
      std::string buf;
      buf.resize(6);

      auto reads = r.read(buf.data(), buf.size());
      EXPECT_EQ(buf, content.substr(offset, reads));
      EXPECT_EQ(reads, buf.size());

      offset += reads;
   }

   /* Round 2 */ {
      // Reads 'from the Meg Project', 20 chars.
      std::string buf;
      buf.resize(20);

      auto reads = r.read(buf.data(), buf.size());
      EXPECT_EQ(buf, content.substr(offset, reads));
      EXPECT_EQ(reads, buf.size());

      offset += reads;
   }

   /* Round 3 */ {
      // Reads '!!!', 3 chars.
      std::string buf;
      buf.resize(3);

      auto reads = r.read(buf.data(), buf.size());
      EXPECT_EQ(buf, content.substr(offset, buf.size()));
      EXPECT_EQ(reads, buf.size());

      offset += reads;
   }

   /*
    * Now, the reader offset in the file
    * should be the content size.
    */
   EXPECT_EQ(offset, content.size());
}

TEST_F(ReaderTest, MethodReadPropertyII) {
   std::string content{"Meguinha"};
   std::ofstream out{temp_file_path.get_string()};
   out << content;
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value())
      << "Unable to get thd temporary test file. Err code: " << (int) f.error();

   std::string buf;
   unsigned reads = 0;
   buf.resize(content.size());

   /*
    * Tries to read 5 chars more the content size.
    * It's expected the method to read only tbe
    * content size.
    */
   {
      filesystem::reader r{f.value()};
      ASSERT_TRUE(r.is_loaded()) << "File isn't loaded.";

      reads = r.read(buf.data(), buf.size() + 5);
      EXPECT_EQ(reads, buf.size());
      EXPECT_EQ(buf, content);
   }

   /* Empty files */

   out.open(temp_file_path.get_string(), std::ios::trunc);
   if (out.is_open()) {
      out.close();
   }

   /*
    * Tries to read from an empty file. It's
    * expected that it reads nothing, obviously.
    */
   {
      filesystem::reader r{f.value()};
      ASSERT_TRUE(r.is_loaded()) << "File isn't loaded.";

      reads = r.read(buf.data(), buf.size());
      EXPECT_EQ(reads, 0);
   }
}

TEST_F(ReaderTest, MethodReadPropertyIII) {
   // TODO: This test it's not too safe.

   unsigned reads = 0;
   std::vector<char> cmpbuf(16, 'M');
   std::vector<char> buf(cmpbuf);

   // Creates temporary test file.
   std::ofstream out{temp_file_path.get_string()};
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value())
      << "Unable to get the temporary test file. Err code: " << (int) f.error();

   // The file should be deleted now.
   std::filesystem::remove(temp_file_path.get_string());
   ASSERT_FALSE(std::filesystem::exists(temp_file_path.get_string()))
      << "Could not remove the temporary test file.";

   filesystem::reader r{f.value()};
   ASSERT_FALSE(r.is_loaded())
      << "Temporary test file must not be loaded now.";

   reads = r.read(buf.data(), 4);
   EXPECT_EQ(reads, 0);
   EXPECT_EQ(buf, cmpbuf);
}

TEST_F(ReaderTest, MethodReadLinePropertyI) {
   std::string ln1{"Multiline text"};
   std::string ln2{"to test read_line() method."};
   std::string ln3{"Um beijo, Meguinha!"};
   std::string ln4{" - Elizeu."};

   std::ofstream out{temp_file_path.get_string()};
   out << ln1;
   out << ln2;
   out << ln3;
   out << ln4;
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value())
      << "Unable to load temporary test file. Err code: " << (int) f.error();

   filesystem::reader r{f.value()};
   ASSERT_TRUE(r.is_loaded())
      << "Could not load temporary test file.";

   std::string buf(32, 0);
   unsigned reads = 0, offset = 0;

   /* Reads the first line */ {
      buf.resize(ln1.size());
      reads = r.read_line(buf.data(), buf.size());

      EXPECT_EQ(reads, ln1.size());
      EXPECT_EQ(buf, ln1);
      offset += reads;
   }

   /* Reads the second line */ {
      buf.resize(ln2.size());
      reads = r.read_line(buf.data(), buf.size());

      EXPECT_EQ(reads, ln2.size());
      EXPECT_EQ(buf, ln2);
      offset += reads;
   }

   /* Reads from the 5th column of the third line */ {
      constexpr size_t SKIPSZ = 10;
      buf.resize(ln3.size() - SKIPSZ);
      r.seek(offset + SKIPSZ, false);
      reads = r.read_line(buf.data(), buf.size());

      EXPECT_EQ(reads, ln3.size() - SKIPSZ);
      EXPECT_EQ(buf, ln3.substr(SKIPSZ));
      offset += reads;
   }

   /* Reads the fourth line */ {
      buf.resize(ln4.size());
      reads = r.read_line(buf.data(), buf.size());

      EXPECT_EQ(reads, ln4.size());
      EXPECT_EQ(buf, ln4);
   }
}

TEST_F(ReaderTest, MethodReadLinePropertyII) {
   std::string content{"line1\nline2"};
   std::ofstream out{temp_file_path.get_string()};
   out << content;
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value())
      << "Unable to load temporary test file. Err code: " << (int) f.error();

   filesystem::reader r{f.value()};
   ASSERT_TRUE(r.is_loaded())
      << "Could not load temporary test file.";

   std::string buf;
   unsigned linesz = 0;

   /* Gets the size of the 1st line */ {
      linesz = r.read_line(nullptr, 0);
      /* Line size is 5. */
      EXPECT_EQ(linesz, 5);
   }
}

TEST_F(ReaderTest, MethodReadLinePropertyIII) {
   // TODO: This test it's not too safe.

   // Creates temporary test file.
   std::ofstream out{temp_file_path.get_string()};
   out << "The text.";
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value()) << "Unable to get the temporary test file. Err code: " << (int) f.error();

   /* File can be loaded */ {
      filesystem::reader r{f.value()};
      EXPECT_TRUE(r.is_loaded());
   }

   // The file should be deleted now.
   std::filesystem::remove(temp_file_path.get_string());
   ASSERT_FALSE(std::filesystem::exists(temp_file_path.get_string()))
      << "Could not remove the temporary test file.";

   /* File can't be loaded now. */ {
      filesystem::reader r{f.value()};
      EXPECT_FALSE(r.is_loaded());
   }
}

TEST_F(ReaderTest, MethodIsLoadedPropertyIII) {
   // TODO: This test it's not too safe.

   std::vector<char> cmpbuf(16, 'M');
   std::vector<char> buf(cmpbuf);

   // Creates temporary test file.
   std::ofstream out{temp_file_path.get_string()};
   out << "The text.";
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value())
      << "Unable to get the temporary test file. Err code: " << (int) f.error();

   /* Call with offset param greater than file size returns `false` */ {
      filesystem::reader r{f.value()};
      ASSERT_TRUE(r.is_loaded())
         << "Unable to load temporary test file.";

      EXPECT_FALSE(r.seek(f.value().size + 1, false));
      EXPECT_FALSE(r.seek(f.value().size + 1, true));
   }

   // The file should be deleted now.
   std::filesystem::remove(temp_file_path.get_string());
   ASSERT_FALSE(std::filesystem::exists(temp_file_path.get_string()))
      << "Could not remove the temporary test file.";

   /* Call with unloaded file returns `false` */ {
      filesystem::reader r{f.value()};
      ASSERT_FALSE(r.is_loaded())
         << "Temporary test file must not be loaded now.";

      EXPECT_FALSE(r.seek(0 /* Vald offset */, false));
      EXPECT_FALSE(r.seek(0 /* Valid offset */, true));

      EXPECT_FALSE(r.seek(f.value().size + 1 /* Invalid offset */, false));
      EXPECT_FALSE(r.seek(f.value().size + 1 /* Invalid offset */, true));
   }
}

TEST_F(ReaderTest, MethodSeekPropertyI) {
   std::string content{"Some words to read."};

   std::ofstream out{temp_file_path.get_string()};
   out << content;
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value())
      << "Temporary test file not found. Err code: " << (int) f.error();

   filesystem::reader r{f.value()};
   ASSERT_TRUE(r.is_loaded())
      << "Unable to load temporary test file.";

   unsigned reads = 0;
   unsigned offset = 0;

   /* Offset should be at the 'Some' word (pos 0) */ {
      std::string buf(4, 0);
      reads = r.read(buf.data(), buf.size());

      EXPECT_EQ(reads, buf.size());
      EXPECT_EQ(buf, content.substr(0, buf.size()));

      offset += reads;
   }

   /* Offset should be at one after te 'words' word (pos 4) */ {
      /*
       * Tries to move the offset one more character.
       * Afterwards, the offset will be exacly at the
       * 'words' word of the file.
       */
      offset++;
      r.seek(offset, false);

      std::string buf(4, 0);
      reads = r.read(buf.data(), buf.size());

      EXPECT_EQ(reads, buf.size());
      EXPECT_EQ(buf, content.substr(offset, buf.size()));
   }
}

TEST_F(ReaderTest, MethodSeekPropertyII) {
   std::string content{"Some words to read."};

   std::ofstream out{temp_file_path.get_string()};
   out << content;
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value())
      << "Temporary test file not found. Err code: " << (int) f.error();

   filesystem::reader r{f.value()};
   ASSERT_TRUE(r.is_loaded())
      << "Unable to load temporary test file.";

   unsigned reads = 0;
   unsigned offset = 0;

   /* Offset should be at the 'Some' word (pos 0) */ {
      std::string buf(4, 0);
      reads = r.read(buf.data(), buf.size());

      EXPECT_EQ(reads, buf.size());
      EXPECT_EQ(buf, content.substr(0, buf.size()));

      offset += reads;
   }

   /* Offset should be at one after te 'words' word (pos 4) */ {
      /*
       * Tries to move the offset 5 bhtes from
       * from the end of the file. Afterwards,
       * the offset will be exacly at the 'read'
       * word.
       */
      r.seek(5, true);
      offset = content.size() - 5;  // Expected offset.

      std::string buf(5, 0);
      reads = r.read(buf.data(), buf.size());

      EXPECT_EQ(reads, buf.size());
      EXPECT_EQ(buf, content.substr(offset, 5));
   }
}

TEST_F(ReaderTest, MethodSeekPropertyIII) {
   // TODO: This test it's not too safe.

   std::vector<char> cmpbuf(16, 'M');
   std::vector<char> buf(cmpbuf);

   // Creates temporary test file.
   std::ofstream out{temp_file_path.get_string()};
   out << "The text.";
   out.close();

   auto f = filesystem::get_file(temp_file_path);
   ASSERT_TRUE(f.has_value())
      << "Unable to get the temporary test file. Err code: " << (int) f.error();

   /* Call with offset param greater than file size returns `false` */ {
      filesystem::reader r{f.value()};
      ASSERT_TRUE(r.is_loaded())
         << "Unable to load temporary test file.";

      EXPECT_FALSE(r.seek(f.value().size + 1, false));
      EXPECT_FALSE(r.seek(f.value().size + 1, true));
   }

   // The file should be deleted now.
   std::filesystem::remove(temp_file_path.get_string());
   ASSERT_FALSE(std::filesystem::exists(temp_file_path.get_string()))
      << "Could not remove the temporary test file.";

   /* Call with unloaded file returns `false` */ {
      filesystem::reader r{f.value()};
      ASSERT_FALSE(r.is_loaded())
         << "Temporary test file must not be loaded now.";

      EXPECT_FALSE(r.seek(0 /* Vald offset */, false));
      EXPECT_FALSE(r.seek(0 /* Valid offset */, true));

      EXPECT_FALSE(r.seek(f.value().size + 1 /* Invalid offset */, false));
      EXPECT_FALSE(r.seek(f.value().size + 1 /* Invalid offset */, true));
   }
}

int main(int argc, char *argv[]) {
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
