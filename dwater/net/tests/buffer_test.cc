// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        buffer_test.cc
// Descripton:       

#include "dwater/net/buffer.h"

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using dwater::string;
using dwater::net::Buffer;
BOOST_AUTO_TEST_CASE(testBufferAppendRetrieve)
{
  Buffer buf;
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend);

  const string str(200, 'x');
  buf.Append(str);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), str.size());
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size - str.size());
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend);

  const string str2 =  buf.RetrieveAsString(50);
  BOOST_CHECK_EQUAL(str2.size(), 50);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), str.size() - str2.size());
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size - str.size());
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend + str2.size());
  BOOST_CHECK_EQUAL(str2, string(50, 'x'));

  buf.Append(str);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 2*str.size() - str2.size());
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size - 2*str.size());
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend + str2.size());

  const string str3 =  buf.RetrieveAllAsString();
  BOOST_CHECK_EQUAL(str3.size(), 350);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend);
  BOOST_CHECK_EQUAL(str3, string(350, 'x'));
}

BOOST_AUTO_TEST_CASE(testBufferGrow)
{
  Buffer buf;
  buf.Append(string(400, 'y'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 400);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size-400);

  buf.Retrieve(50);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 350);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size-400);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend+50);

  buf.Append(string(1000, 'z'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 1350);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend+50); // FIXME

  buf.RetrieveAll();
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), 1400); // FIXME
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend);
}

BOOST_AUTO_TEST_CASE(testBufferInsideGrow)
{
  Buffer buf;
  buf.Append(string(800, 'y'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 800);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size-800);

  buf.Retrieve(500);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 300);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size-800);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend+500);

  buf.Append(string(300, 'z'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 600);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size-600);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend);
}

BOOST_AUTO_TEST_CASE(testBufferShrink)
{
  Buffer buf;
  buf.Append(string(2000, 'y'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 2000);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend);

  buf.Retrieve(1500);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 500);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend+1500);

  buf.Shrink(0);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 500);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size-500);
  BOOST_CHECK_EQUAL(buf.RetrieveAllAsString(), string(500, 'y'));
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend);
}

BOOST_AUTO_TEST_CASE(testBufferPrepend)
{
  Buffer buf;
  buf.Append(string(200, 'y'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 200);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size-200);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend);

  int x = 0;
  buf.Prepend(&x, sizeof x);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 204);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size-200);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kcheap_prepend - 4);
}

BOOST_AUTO_TEST_CASE(testBufferReadInt)
{
  Buffer buf;
  buf.Append("HTTP");

  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 4);
  BOOST_CHECK_EQUAL(buf.PeekInt8(), 'H');
  int top16 = buf.PeekInt16();
  BOOST_CHECK_EQUAL(top16, 'H'*256 + 'T');
  BOOST_CHECK_EQUAL(buf.PeekInt32(), top16*65536 + 'T'*256 + 'P');

  BOOST_CHECK_EQUAL(buf.ReadInt8(), 'H');
  BOOST_CHECK_EQUAL(buf.ReadInt16(), 'T'*256 + 'T');
  BOOST_CHECK_EQUAL(buf.ReadInt8(), 'P');
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kinitial_size);

  buf.AppendInt8(-1);
  buf.AppendInt16(-2);
  buf.AppendInt32(-3);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 7);
  BOOST_CHECK_EQUAL(buf.ReadInt8(), -1);
  BOOST_CHECK_EQUAL(buf.ReadInt16(), -2);
  BOOST_CHECK_EQUAL(buf.ReadInt32(), -3);
}

BOOST_AUTO_TEST_CASE(testBufferFindEOL)
{
  Buffer buf;
  buf.Append(string(100000, 'x'));
  const char* null = NULL;
  BOOST_CHECK_EQUAL(buf.FindEOL(), null);
  BOOST_CHECK_EQUAL(buf.FindEOL(buf.Peek()+90000), null);
}

void output(Buffer&& buf, const void* inner)
{
  Buffer newbuf(std::move(buf));
  // printf("New Buffer at %p, inner %p\n", &newbuf, newbuf.Peek());
  BOOST_CHECK_EQUAL(inner, newbuf.Peek());
}

// NOTE: This test fails in g++ 4.4, passes in g++ 4.6.
BOOST_AUTO_TEST_CASE(testMove)
{
  Buffer buf;
  buf.Append("dwater", 5);
  const void* inner = buf.Peek();
  // printf("Buffer at %p, inner %p\n", &buf, inner);
  output(std::move(buf), inner);
}

