// point_iterator_code
// keywords:
// * index
// * topology_opaque_pointer
// * body
// * input_data_handle
// * output_data_handle
PointIterator iterator_$index$ = point_iterator($topology_opaque_pointer$);
for (begin_point(&iterator_$index$); !is_done_point(&iterator_$index$);
  next_point(&iterator_$index$))
{
  float input_temp;
  float output_temp;
  input_temp = *get_data_reference_point(&iterator_$index$,
    &$input_data_handle$);

  $body$

  *get_data_reference_point(&iterator_$index$,
    &$output_data_handle$) = output_temp;
}