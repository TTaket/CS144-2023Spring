#include "tcp_receiver.hh"

using namespace std;
TCPReceiver::TCPReceiver() : ISN( 0 ), SYN_flag( false ), FIN_flag( false ), checkpoint( 0 ) {};
void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // struct TCPSenderMessage
  // {
  //   Wrap32 seqno { 0 };
  //   bool SYN { false };
  //   Buffer payload {};
  //   bool FIN { false };

  //   // How many sequence numbers does this segment use?
  //   size_t sequence_length() const { return SYN + payload.size() + FIN; }
  // };

  if ( !SYN_flag && message.SYN ) { // 只接受第一个SYN
    ISN = message.seqno;
  } else {
    if ( !SYN_flag )
      return; // 没有收到SYN就拒收
  }

  reassembler.insert( message.seqno.unwrap( ISN, checkpoint ), message.payload, message.FIN, inbound_stream );
  checkpoint = inbound_stream.bytes_pushed();

  if ( !SYN_flag && message.SYN ) {
    SYN_flag = true;
    reassembler.insert_SYN();
  }

  if ( inbound_stream.is_closed() && !FIN_flag ) {
    FIN_flag = true;
    reassembler.insert_FIN();
  }
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // struct TCPReceiverMessage
  // {
  //   std::optional<Wrap32> ackno {};
  //   uint16_t window_size {};
  // };
  TCPReceiverMessage ret;
  if ( SYN_flag )
    ret.ackno = Wrap32( Wrap32::wrap( inbound_stream.bytes_pushed(), ISN ).getrawvalue() ) + uint32_t( SYN_flag )
                + uint32_t( FIN_flag );
  ret.window_size = (uint16_t)min( inbound_stream.available_capacity(), (uint64_t)UINT16_MAX );
  return { ret };
}
