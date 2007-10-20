package Search::Xapian::Weight;

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

=head1 NAME

Search::Xapian::Weight - base class for Weighting schemes.

=head1 DESCRIPTION

This is an asbtract base class for weighting schemes in Xapian.

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::BoolWeight>,L<Search::Xapian::BM25Weight>

=cut

1;
