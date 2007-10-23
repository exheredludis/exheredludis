/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/sha256.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for paludis::digests::SHA256.
 *
 */

namespace
{
    /**
     * \name Test utilities
     * \{
     */

    unsigned char dehex_c(unsigned char c)
    {
        if (c >= '0' && c <= '9')
            return c - '0';
        else if (c >= 'a' && c <= 'f')
            return c + 10 - 'a';
        else
            throw "meh!";
    }

    std::string dehex(const std::string & s)
    {
        std::string result;
        std::string::size_type p(0);
        while (p < s.length())
        {
            unsigned char c;
            c = (dehex_c(s.at(p)) << 4) + dehex_c(s.at(p + 1));
            result.append(1, c);
            p += 2;
        }
        return result;
    }

    /**
     * \}
     */
}

namespace test_cases
{
    /**
     * \name Test cases for paludis::digests::SHA256
     * \{
     */

    struct SHA256TestCase : TestCase
    {
        std::string data;
        std::string expected;

        SHA256TestCase(const std::string & s, const std::string & d,
                const std::string & e) :
            TestCase("sha256 " + s),
            data(d),
            expected(e)
        {
        }

        void run()
        {
            std::stringstream ss(data);
            SHA256 s(ss);
            TEST_CHECK_EQUAL(s.hexsum(), expected);
        }
    };

    SHA256TestCase t1("empty", "",
            "e3b0c442""98fc1c14""9afbf4c8""996fb924"
            "27ae41e4""649b934c""a495991b""7852b855");
    SHA256TestCase t2("abc", "abc",
            "ba7816bf""8f01cfea""414140de""5dae2223"
            "b00361a3""96177a9c""b410ff61""f20015ad");
    SHA256TestCase t3("abcd...", "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "248d6a61""d20638b8""e5c02693""0c3e6039"
            "a33ce459""64ff2167""f6ecedd4""19db06c1");

    /*
     * the following tests are from:
     *     http://csrc.ncsl.nist.gov/cryptval/
     */

    SHA256TestCase t_11_SHA256ShortMsg("t 11 SHA256ShortMsg",
            dehex("bd"), "68325720aabd7c82f30f554b313d0570c95accbb"
            "7dc4b5aae11204c08ffe732b");

    SHA256TestCase t_14_SHA256ShortMsg("t 14 SHA256ShortMsg",
            dehex("5fd4"), "7c4fbf484498d21b487b9d61de8914b2eadaf269"
            "8712936d47c3ada2558f6788");

    SHA256TestCase t_17_SHA256ShortMsg("t 17 SHA256ShortMsg",
            dehex("b0bd69"), "4096804221093ddccfbf46831490ea63e9e99414"
            "858f8d75ff7f642c7ca61803");

    SHA256TestCase t_20_SHA256ShortMsg("t 20 SHA256ShortMsg",
            dehex("c98c8e55"), "7abc22c0ae5af26ce93dbb94433a0e0b2e119d01"
            "4f8e7f65bd56c61ccccd9504");

    SHA256TestCase t_23_SHA256ShortMsg("t 23 SHA256ShortMsg",
            dehex("81a723d966"), "7516fb8bb11350df2bf386bc3c33bd0f52cb4c67"
            "c6e4745e0488e62c2aea2605");

    SHA256TestCase t_26_SHA256ShortMsg("t 26 SHA256ShortMsg",
            dehex("c97a2db566e5"), "0eb0281b27a4604709b0513b43ad29fdcff9a7a9"
            "58554abc689d7fe35af703e4");

    SHA256TestCase t_29_SHA256ShortMsg("t 29 SHA256ShortMsg",
            dehex("f53210aa6ed72e"), "dee684641421d1ba5a65c71f986a117cbb3d619a"
            "052a0b3409306c629575c00f");

    SHA256TestCase t_32_SHA256ShortMsg("t 32 SHA256ShortMsg",
            dehex("0df1cd526b5a4edd"), "47f527210d6e8f940b5082fec01b7305908fa2b4"
            "9ea3ae597c19a3986097153c");

    SHA256TestCase t_35_SHA256ShortMsg("t 35 SHA256ShortMsg",
            dehex("b80233e2c53ab32cc3"), "c60d239cc6da3ad31f4de0c2d58a73ccf3f9279e"
            "504fa60ad55a31dcf686f3ca");

    SHA256TestCase t_38_SHA256ShortMsg("t 38 SHA256ShortMsg",
            dehex("5d54ed5b52d879aeb5dd"), "e0164d90dbfcf173bb88044fac596ccd03b8d247"
            "c79907aaa5701767fad7b576");

    SHA256TestCase t_41_SHA256ShortMsg("t 41 SHA256ShortMsg",
            dehex("df866ecb67ab00515f6247"), "dc990ef3109a7bcf626199db9ab7801213ceb0ad"
            "2ee398963b5061e39c05c7b5");

    SHA256TestCase t_44_SHA256ShortMsg("t 44 SHA256ShortMsg",
            dehex("0757de9485a2eaea51126077"), "c1c9a4daadcc8678835872c7f1f8824376ac7b41"
            "2e1fc2285069b41afd51397e");

    SHA256TestCase t_47_SHA256ShortMsg("t 47 SHA256ShortMsg",
            dehex("7c66f5d443c11cfb39dd0aa715"), "6840619417b4d8ecaa7902f8eaf2e82be2638dec"
            "97cb7e8fcc377007cc176718");

    SHA256TestCase t_50_SHA256ShortMsg("t 50 SHA256ShortMsg",
            dehex("329624fed35639fe54957b7d47a9"), "0f5308ff22b828e18bd65afbc427e3c1a6789628"
            "32519df5f2f803f68f55e10b");

    SHA256TestCase t_53_SHA256ShortMsg("t 53 SHA256ShortMsg",
            dehex("c34e59652acc043873ecf6a4ab1060"), "0fdf1604ac0d717ec9587b4de5444aaade807589"
            "d90eb326eaf6acb58a051e79");

    SHA256TestCase t_56_SHA256ShortMsg("t 56 SHA256ShortMsg",
            dehex("fdf4700984ee11b70af1880d0e0fefd4"), "b01ae16eed3b4a770f127b98469ba26fe3d8e9f5"
            "9d8a2983214afe6cff0e6b6c");

    SHA256TestCase t_59_SHA256ShortMsg("t 59 SHA256ShortMsg",
            dehex("ea40aadbefedb0e0d78d067c6cd65c2c87"), "36157bbe61931d58a3a644953eaf131bbc2591c6"
            "73a1f20353f51ca5054fc1c2");

    SHA256TestCase t_62_SHA256ShortMsg("t 62 SHA256ShortMsg",
            dehex("6d1092004670efab3af483d265d8e7b3da73"), "67fbf35d360d72b101410794ccf197106c0e784a"
            "fa9c80206a550b600dbf1f16");

    SHA256TestCase t_65_SHA256ShortMsg("t 65 SHA256ShortMsg",
            dehex("55a10148ae7b09ac4e71df438135bc70e873eb"), "cbe7965513af46dfd596dc5839cb82a5c6c73280"
            "34b1dd0042a9f4b71fb14430");

    SHA256TestCase t_68_SHA256ShortMsg("t 68 SHA256ShortMsg",
            dehex("a03f8fcd777bd933b4b0af8c5ce3d61308565649"), "ddfce4e8c7b38845e2a81b7fc27a06366467a9e1"
            "11316014013f9701e2413ce0");

    SHA256TestCase t_71_SHA256ShortMsg("t 71 SHA256ShortMsg",
            dehex("8e5d6cba8d4b206381e33ca7339bec504f3d6119"
                "ba"), "92f678a3e59d0dd3610eec3222b8c6ebd28eead5"
            "30723fbd226747534da22b6c");

    SHA256TestCase t_74_SHA256ShortMsg("t 74 SHA256ShortMsg",
            dehex("96db1b62eed85f2628d0c25da534401fe80d13d0"
                "9beb"), "725bab4457c789d6a4cc4736b9c2c662cda18407"
            "150844d74d6aa4efd72dbb05");

    SHA256TestCase t_77_SHA256ShortMsg("t 77 SHA256ShortMsg",
            dehex("1c482a45dfbcda549729126b533477edfaf7476f"
                "de498f"), "6523f24f225b996aad1a8b317e6e0f8e97673dcf"
            "f3fd62a27ff9f3888ea1302d");

    SHA256TestCase t_80_SHA256ShortMsg("t 80 SHA256ShortMsg",
            dehex("0f677d8e4c6d6a057492670d99adb870adf68a36"
                "ead37919"), "44acbbc6b48bf37ee088b9c8546fc46e5a5f0d63"
            "7b5e444f628de186144087fd");

    SHA256TestCase t_83_SHA256ShortMsg("t 83 SHA256ShortMsg",
            dehex("c09056d597816542bffe4bb33e475dfb2d629301"
                "6906ddc18c"), "f4baeaef70588a0820d63c2401dd84f98adf7366"
            "782d196f8698d7dfd3db1c29");

    SHA256TestCase t_86_SHA256ShortMsg("t 86 SHA256ShortMsg",
            dehex("72f313fdcf52d0749c9937cc2e53f50b44d65a54"
                "4876bab7d2f8"), "cfa67aa52fd675fca985f69f9ca58af62baead8c"
            "39723bb6bfbae8a5d4bb9beb");

    SHA256TestCase t_89_SHA256ShortMsg("t 89 SHA256ShortMsg",
            dehex("09f6fe6cbe6744149f792a4a827e4e8909627abf"
                "75301bf7bbd7f5"), "657633891dc6274d6aeda78e7313dfb960eac9a2"
            "4d29293a057b9746a18de4ec");

    SHA256TestCase t_92_SHA256ShortMsg("t 92 SHA256ShortMsg",
            dehex("9e1cfeb335bc331744247df4bbd56876a7f69298"
                "aaf6b9e7a8731889"), "930058dd21cb48b2cf90eaca55322ddf48582687"
            "838a584928440504a2fde578");

    SHA256TestCase t_95_SHA256ShortMsg("t 95 SHA256ShortMsg",
            dehex("b8913001efb1b7f4bd975e349c5b2cbe66045bf0"
                "d2fb019b3bc0f059a4"), "a0eb0b7fad1d1b6de4f9096724a621720538a9c3"
            "f2f6d11134d68cb9ee52fc88");

    SHA256TestCase t_98_SHA256ShortMsg("t 98 SHA256ShortMsg",
            dehex("8f08537d50928c911a68b071d65b9e8f038264d3"
                "b62c5f33de18a484cde9"), "10aad5cd4484387373577a881974f1a550782108"
            "bc88b4e2e8085e9c3e938bbb");

    SHA256TestCase t_101_SHA256ShortMsg("t 101 SHA256ShortMsg",
            dehex("fd846162c4da936d004ffe0cbe844d940f1c2953"
                "157cf4765dceba2a6f4c64"), "c13ba769aea0e478816f2f608b5cec3fe14672ea"
            "033088a8641cfe69b4ff57cb");

    SHA256TestCase t_104_SHA256ShortMsg("t 104 SHA256ShortMsg",
            dehex("8cf53d90077df9a043bf8d10b470b144784411c9"
                "3a4d504556834dae3ea4a5bb"), "56059e8cb3c2978b198208bf5ca1e1ea5659b737"
            "a506324b7cec75b5ebaf057d");

    SHA256TestCase t_107_SHA256ShortMsg("t 107 SHA256ShortMsg",
            dehex("1bbc2b15253c126e301f9f64b97be4ce13e96337"
                "687e2e78fbfd4c8daf4a5fa1cd"), "d973b5dcdae4cf2599f4db4068e4aa354f22d890"
            "1adc463ca3938c465578147b");

    SHA256TestCase t_110_SHA256ShortMsg("t 110 SHA256ShortMsg",
            dehex("c1bdb3bfc65dfe9a393331266c58d05fb9c8b747"
                "6bb717dadc29bc43dabd91504fc9"), "57844e1d762e6b7bb86dbfcc5c5a59578d39cc66"
            "5d1ddbe4de03a61778061af1");

    SHA256TestCase t_113_SHA256ShortMsg("t 113 SHA256ShortMsg",
            dehex("26eb621a45bd9c9c764ccbb672b99f2a8379c7bb"
                "f4fb07eec58a8b0ea4747b72196ccf"), "73dc27bd45daccd0f811381230cf7f2a1d3ed120"
            "2e9a770af733146b1e166315");

    SHA256TestCase t_116_SHA256ShortMsg("t 116 SHA256ShortMsg",
            dehex("7e3e3986109162e0c56357048bbd86ff49b93644"
                "b7fb064e7280968650978466f02c9adf"), "682c474799f5103252c3e2efef7f747783e514b5"
            "4e93b8303b0e07ee4218f78e");

    SHA256TestCase t_119_SHA256ShortMsg("t 119 SHA256ShortMsg",
            dehex("763c1a9ea50bd72bfc516989ddf3eff2f208f64f"
                "ccea3cf0ca8dba7f3d10e237c99226510f"), "54d6cb2b09825eab064c8952113b9897a3344737"
            "cd186a8e6be0a0b258da3e57");

    SHA256TestCase t_122_SHA256ShortMsg("t 122 SHA256ShortMsg",
            dehex("e1a7ffea8417e7cd49b96e355fd44f3f7a150fab"
                "6dd8343dfba3b262eaf3a6175a3c4607552b"), "83baa80caade404c446833ecef2e595bba6dce2c"
            "b7f7422fad2972a9fe327aca");

    SHA256TestCase t_125_SHA256ShortMsg("t 125 SHA256ShortMsg",
            dehex("692a18effad8317a11a5cddb917f7389e1be6dba"
                "34572a300e52e056047e758bc363a0be53784c"), "0c0c6a6b27a6d7a7a5130d70db3b8bc1bd8001d1"
            "03efe72f45b082cadbd03742");

    SHA256TestCase t_128_SHA256ShortMsg("t 128 SHA256ShortMsg",
            dehex("73fda1e1cb7dc9a9ece858d040d7105cc126eab1"
                "53fb0bb55703f4317dfff97bd980f4523aee3a09"), "9878f8804e00828b39261843f2b3eda19a7e9b9f"
            "f4cc2e23f7ea1f62f4491ff2");

    SHA256TestCase t_131_SHA256ShortMsg("t 131 SHA256ShortMsg",
            dehex("2321d88c19e3e6a8309a09a5428c01991e164468"
                "23f13b2f0db4ade30e9a7c3521868fb99b440f48"
                "02"), "f1bd3a8a74c8f0093038499ef63794d86fc6d826"
            "02a802a435718e61e7b396cc");

    SHA256TestCase t_134_SHA256ShortMsg("t 134 SHA256ShortMsg",
            dehex("b9eaebda29172b052bcc1e3a9c7f2eced43c084a"
                "86f89f61e7237425137c167aac29e4cac4071afa"
                "fd3f"), "ea43ec91285145d8f29915b227a0e35c89f90d96"
            "8f9a14332dad275cfd52d619");

    SHA256TestCase t_137_SHA256ShortMsg("t 137 SHA256ShortMsg",
            dehex("332daf07d3a6775b18572549a6e12b8a27d81b7c"
                "4abcc5bd0b2b9ff936546b0026af131cd3ecd8a1"
                "0c29ab"), "a573959ba6b1c3bebfd6288c806b72a65650d23b"
            "d46d123816a2a6a0e47d1e66");

    SHA256TestCase t_140_SHA256ShortMsg("t 140 SHA256ShortMsg",
            dehex("30ac7eace1f2e41034c25a3d3e2db979c23dfaa7"
                "a4914b0da147625b3e1f12e9fedc1c41d8ee47dd"
                "e84fb332"), "c0c3f40d34e711bfadf517b3a78140e379fba5f7"
            "edf2c1bc3ce82469dae4d2d5");

    SHA256TestCase t_143_SHA256ShortMsg("t 143 SHA256ShortMsg",
            dehex("02c3964c4ad9c4af97d373099302c2cd770ad06c"
                "7d8bd11c970161d861e917a854265e223da28031"
                "ee38041534"), "c13c622bf08a3d3cf1fd6fa5e26e505e551b1643"
            "bc5a0f59ed29541235218f77");

    SHA256TestCase t_146_SHA256ShortMsg("t 146 SHA256ShortMsg",
            dehex("b9eed82edcf0c7ba69f6f6ac5722cb61daecaf30"
                "437511582117ad36ad410ebc6582511ef6e32dce"
                "5f7a30ab543c"), "6ac64caaeda4763d28a44b363823a6b819285410"
            "fb4162af6ca657396f6028d0");

    SHA256TestCase t_149_SHA256ShortMsg("t 149 SHA256ShortMsg",
            dehex("b574865024828bf651df070ac0cec1849aa64709"
                "01d2e30fa01dcb43862d9827344cf900f46fa9ef"
                "6d709e5e759f84"), "4c839e8f8f373c25a9a3351257c6152258ff8e6a"
            "88dad42f30f2bbecab56c20b");

    SHA256TestCase t_152_SHA256ShortMsg("t 152 SHA256ShortMsg",
            dehex("eebcf5cd6b12c90db64ff71a0e08ccd956e170a5"
                "0dad769480d6b1fb3eff4934cde90f9e9b930ee6"
                "37a66285c10f4e8a"), "c117b9dce689c399ec99008788cd5d24d8396fab"
            "7d96315c4f3fe6d56da63bb3");

    SHA256TestCase t_155_SHA256ShortMsg("t 155 SHA256ShortMsg",
            dehex("1b7a73770d168da45bf2e512eee45153e02f4dfe"
                "3b42e50304a3d63d7826f0469562be8fdc6569b0"
                "56a7dafcd53d1f597c"), "0b42cfc3dd3d3198f06c30e087837ec6a6dd35d0"
            "8e54e886c682709f8f42457a");

    SHA256TestCase t_158_SHA256ShortMsg("t 158 SHA256ShortMsg",
            dehex("0072ae2f3bda67736b9c66e2130260b3a4847bc3"
                "968e037cb6835efcc2014273336725cd5a94f592"
                "aef20a0a65b459a4415b"), "217cf25b8b343c28336b1c1e9bed29e0c96045bc"
            "93daf426e490b608b0905c90");

    SHA256TestCase t_161_SHA256ShortMsg("t 161 SHA256ShortMsg",
            dehex("2ac748680f3bc1bf098c4be38c7194643b0d009e"
                "51c43630404cdfaf9807aa9b299094916c9466c3"
                "1fe37fa630c6d3eadc9434"), "3ea59e2e79513679a22e962f22408306f7e8f6e5"
            "62c2f1f210e279fad8eaacc6");

    SHA256TestCase t_164_SHA256ShortMsg("t 164 SHA256ShortMsg",
            dehex("893d1a8863d234ee50e5a8c7650a4de047230ad0"
                "3d268dde8921401ff97b79dfb97cf2426b0f782b"
                "79c7e75daa2155e1f4098ea7"), "f7808e03e5d5af43c2bffb66e35d1ecbd79f4d8f"
            "ec44f821f73a235d17c70a89");

    SHA256TestCase t_167_SHA256ShortMsg("t 167 SHA256ShortMsg",
            dehex("cf673b96eaf241cfa3e262dc6fe65f08bcc2be56"
                "d8a2c9710eaddae212ded6859f0ff83e5e57d0e8"
                "0a968b8ed24e74defeb5bbdad6"), "9bdb7cf0492ace4620a47660acd127f951767b07"
            "38b5504451d6ed56e4fa3cbd");

    SHA256TestCase t_170_SHA256ShortMsg("t 170 SHA256ShortMsg",
            dehex("0d545be1f47b966214691c21278704e89a17d52d"
                "d96aeeeacc5325a9a1ddafdecd39407a4dfa72bd"
                "32856b4c5cc2ba838618830c8399"), "ad53e0db7e63211c8b00947908ce29660c4376e2"
            "44e19cd30a659af65dc6f1fe");

    SHA256TestCase t_173_SHA256ShortMsg("t 173 SHA256ShortMsg",
            dehex("9eabfcd3603337df3dcd119d6287a9bc8bb94d65"
                "0ef29bcf1b32e60d425adc2a35e06577d0c7ce24"
                "56cf260efee9e8d8aeeddb3d068f37"), "83eeed2dfeb8d2604ab5ec1ac9b5dcab8cc22225"
            "18468bc5c24c16ce72e70687");

    SHA256TestCase t_176_SHA256ShortMsg("t 176 SHA256ShortMsg",
            dehex("2fc7b9e8b8dcaac64ecef4c5f91877543ac36ae4"
                "94d9faf84b1d347b6cf925570db84043d6f500dc"
                "c153cef81d6f2437d913f3dbffad42d9"), "6ef7e9f12267ebc4901267da147effdcdebcd6ec"
            "5393c7f62ec4c4f06ca72649");

    SHA256TestCase t_179_SHA256ShortMsg("t 179 SHA256ShortMsg",
            dehex("cf95929ab732f9ef5e8c3e6b4ed753852ee74e4f"
                "ddf31b56c29a6ec95d23fcde2209eb7288b787f0"
                "5d9036735c32ae2f01fc650d9cce4995a5"), "3e5854169da065407fa465a4694f3fcb1d141480"
            "a8f84c970a0f63364ec8f590");

    SHA256TestCase t_182_SHA256ShortMsg("t 182 SHA256ShortMsg",
            dehex("826378013988684c40f4d917c7ed8b72aba66fd6"
                "8f085d0b2eb20948ef3f349dbbc71f8e0ba84501"
                "4586495a48902ee44505c673d2f76d473950"), "5b506b823ef6658939aca22f52bbe5a4b849c31b"
            "8fa1d09139352e501137bc04");

    SHA256TestCase t_185_SHA256ShortMsg("t 185 SHA256ShortMsg",
            dehex("0cab6d38ce9849fcbd589f7235a6d2c2cb933e26"
                "e1ca6f4e78189104452c280c069b024e16276937"
                "3f409d5cd0cb8160f0239418325d23ee6ad1bd"), "92943076cda4c46718e55df64d7580e12b8fb2c2"
            "911e87851246ccf6791fa3e6");

    SHA256TestCase t_188_SHA256ShortMsg("t 188 SHA256ShortMsg",
            dehex("3fb4a8c5b57c14731179256608614c95c9725dda"
                "d5fbfa99111d4fa319d3015ad830601556e8e4c6"
                "d012d7da0e2c4f60f1605f6e4c058ec0f46988a3"), "8e90da3eb146935264576f874fcc5a64b7a90ab6"
            "c8a36c15d855b0179f52f899");

    SHA256TestCase t_191_SHA256ShortMsg("t 191 SHA256ShortMsg",
            dehex("9050a6d002c90f6036c592b0f6b866713e7894d2"
                "9645f4a19e0858b3ebd8078711c26d2601ca104d"
                "962dc6ce6ae92634ee7f3ca6baf8810e2126097a"
                "09"), "03c516677735ae83dbe5a7e4c22c1ac1bfedcd46"
            "e7dd785f8bfe38e148eda632");

    SHA256TestCase t_194_SHA256ShortMsg("t 194 SHA256ShortMsg",
            dehex("d659ec136bacfa0b5c906aabedc93c01c5f1efa3"
                "f370a1432ea8778461703f0c67c454da12bac2da"
                "73b8abb755e5eaf10bddf52f6ca908d61bee80da"
                "0c64"), "fff2852957a0eeb577e73fd7d827f650261dfb9a"
            "8a65f52df4bbbc9b2d0ae50e");

    SHA256TestCase t_197_SHA256ShortMsg("t 197 SHA256ShortMsg",
            dehex("b498555658332b197bc5cb7adc5c1997aabbdcf1"
                "f7ffcc2b6b82eb0f350019d247f8e399c3559d3b"
                "b04eb049f28b344c7989c24db83f839b59028dc8"
                "2fa670"), "bfbbf242f79bff4ae0aafb4ccf69b24fdca4342d"
            "83db1dfd1822c74a9e218e8d");

    SHA256TestCase t_200_SHA256ShortMsg("t 200 SHA256ShortMsg",
            dehex("3592ecfd1eac618fd390e7a9c24b656532509367"
                "c21a0eac1212ac83c0b20cd896eb72b801c4d212"
                "c5452bbbf09317b50c5c9fb1997553d2bbc29bb4"
                "2f5748ad"), "105a60865830ac3a371d3843324d4bb5fa8ec0e0"
            "2ddaa389ad8da4f10215c454");

    /**
     * \}
     */
}

