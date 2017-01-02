#include <iostream>

#include "BMPDumper.h"

/**
 * Virtual method for grid saving for 1D
 */
template<>
void
BMPDumper<GridCoordinate1D>::dumpGrid (Grid<GridCoordinate1D> &grid) const
{
#if PRINT_MESSAGE
  const GridCoordinate1D& size = grid.getSize ();

  grid_coord sx = size.getX ();
  std::cout << "Saving 1D to BMP image. Size: " << sx << "x1. " << std::endl;
#endif /* PRINT_MESSAGE */

  writeToFile (grid);

#if PRINT_MESSAGE
  std::cout << "Saved. " << std::endl;
#endif /* PRINT_MESSAGE */
}

/**
 * Virtual method for grid saving for 2D
 */
template<>
void
BMPDumper<GridCoordinate2D>::dumpGrid (Grid<GridCoordinate2D> &grid) const
{
#if PRINT_MESSAGE
  const GridCoordinate2D& size = grid.getSize ();

  grid_coord sx = size.getX ();
  grid_coord sy = size.getY ();

  std::cout << "Saving 2D to BMP image. Size: " << sx << "x" << sy << ". " << std::endl;
#endif /* PRINT_MESSAGE */

  writeToFile (grid);

#if PRINT_MESSAGE
  std::cout << "Saved. " << std::endl;
#endif /* PRINT_MESSAGE */
}

/**
 * Virtual method for grid saving for 3D
 */
template<>
void
BMPDumper<GridCoordinate3D>::dumpGrid (Grid<GridCoordinate3D> &grid) const
{
//  ASSERT_MESSAGE ("3D Dumper is not implemented.")
#if PRINT_MESSAGE
  const GridCoordinate3D& size = grid.getSize ();

  grid_coord sx = size.getX ();
  grid_coord sy = size.getY ();
  grid_coord sz = size.getZ ();

  std::cout << "Saving 3D to BMP image. Size: " << sx << "x" << sy << "x" << sz << ". " << std::endl;
#endif /* PRINT_MESSAGE */

  writeToFile (grid);

#if PRINT_MESSAGE
  std::cout << "Saved. " << std::endl;
#endif /* PRINT_MESSAGE */
}

/**
 * Save grid to file for specific layer for 1D.
 */
template<>
void
BMPDumper<GridCoordinate1D>::writeToFile (Grid<GridCoordinate1D> &grid, GridFileType dump_type) const
{
  const GridCoordinate1D& size = grid.getSize ();
  grid_coord sx = size.getX ();
  grid_coord sy = 1;

  // Create image for current values and max/min values.
  BMP imageRe;
  imageRe.SetSize (sx, sy);
  imageRe.SetBitDepth (24);

#ifdef COMPLEX_FIELD_VALUES
  BMP imageIm;
  imageIm.SetSize (sx, sy);
  imageIm.SetBitDepth (24);
#endif /* COMPLEX_FIELD_VALUES */

  const FieldPointValue* value0 = grid.getFieldPointValue (0);
  ASSERT (value0);

  FPValue maxPosRe = 0;
  FPValue maxNegRe = 0;

#ifdef COMPLEX_FIELD_VALUES
  FPValue maxPosIm = 0;
  FPValue maxNegIm = 0;
#endif /* COMPLEX_FIELD_VALUES */

  switch (dump_type)
  {
    case CURRENT:
    {
#ifdef COMPLEX_FIELD_VALUES
      maxNegRe = maxPosRe = value0->getCurValue ().real ();
      maxNegIm = maxPosIm = value0->getCurValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
      maxNegRe = maxPosRe = value0->getCurValue ();
#endif /* !COMPLEX_FIELD_VALUES */

      break;
    }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
    case PREVIOUS:
    {
#ifdef COMPLEX_FIELD_VALUES
      maxNegRe = maxPosRe = value0->getPrevValue ().real ();
      maxNegIm = maxPosIm = value0->getPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
      maxNegRe = maxPosRe = value0->getPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

      break;
    }
#if defined (TWO_TIME_STEPS)
    case PREVIOUS2:
    {
#ifdef COMPLEX_FIELD_VALUES
      maxNegRe = maxPosRe = value0->getPrevPrevValue ().real ();
      maxNegIm = maxPosIm = value0->getPrevPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
      maxNegRe = maxPosRe = value0->getPrevPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

      break;
    }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
    default:
    {
      UNREACHABLE;
    }
  }

  // Go through all values and calculate max/min.
  for (grid_iter i = 0; i < grid.getSize ().calculateTotalCoord (); ++i)
  {
    const FieldPointValue* current = grid.getFieldPointValue (i);

    ASSERT (current);

    FPValue valueRe = 0;

#ifdef COMPLEX_FIELD_VALUES
    FPValue valueIm = 0;
#endif /* COMPLEX_FIELD_VALUES */

    switch (dump_type)
    {
      case CURRENT:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getCurValue ().real ();
        valueIm = current->getCurValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getCurValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
      case PREVIOUS:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getPrevValue ().real ();
        valueIm = current->getPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (TWO_TIME_STEPS)
      case PREVIOUS2:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getPrevPrevValue ().real ();
        valueIm = current->getPrevPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getPrevPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
      default:
      {
        UNREACHABLE;
      }
    }

    if (valueRe > maxPosRe)
    {
      maxPosRe = valueRe;
    }
    if (valueRe < maxNegRe)
    {
      maxNegRe = valueRe;
    }

#ifdef COMPLEX_FIELD_VALUES
    if (valueIm > maxPosIm)
    {
      maxPosIm = valueIm;
    }
    if (valueIm < maxNegIm)
    {
      maxNegIm = valueIm;
    }
#endif /* COMPLEX_FIELD_VALUES */
  }

  // Set max (diff between max positive and max negative).
  const FPValue maxRe = maxPosRe - maxNegRe;

#ifdef COMPLEX_FIELD_VALUES
  const FPValue maxIm = maxPosIm - maxNegIm;
#endif /* COMPLEX_FIELD_VALUES */

  // Go through all values and set pixels.
  grid_iter end = grid.getSize().calculateTotalCoord ();
  for (grid_iter iter = 0; iter < end; ++iter)
  {
    // Get current point value.
    const FieldPointValue* current = grid.getFieldPointValue (iter);
    ASSERT (current);

    // Calculate its position from index in array.
    GridCoordinate1D coord = grid.calculatePositionFromIndex (iter);

    // Pixel coordinate.
    grid_iter px = coord.getX ();
    grid_iter py = 0;

    // Get pixel for image.
    FPValue valueRe = 0;

#ifdef COMPLEX_FIELD_VALUES
    FPValue valueIm = 0;
#endif /* COMPLEX_FIELD_VALUES */

    switch (dump_type)
    {
      case CURRENT:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getCurValue ().real ();
        valueIm = current->getCurValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getCurValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
      case PREVIOUS:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getPrevValue ().real ();
        valueIm = current->getPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (TWO_TIME_STEPS)
      case PREVIOUS2:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getPrevPrevValue ().real ();
        valueIm = current->getPrevPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getPrevPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
      default:
      {
        UNREACHABLE;
      }
    }

    RGBApixel pixelRe = BMPhelper.getPixelFromValue (valueRe, maxNegRe, maxRe);

#ifdef COMPLEX_FIELD_VALUES
    RGBApixel pixelIm = BMPhelper.getPixelFromValue (valueIm, maxNegIm, maxIm);
#endif /* COMPLEX_FIELD_VALUES */

    // Set pixel for current image.
    imageRe.SetPixel(px, py, pixelRe);

#ifdef COMPLEX_FIELD_VALUES
    imageIm.SetPixel(px, py, pixelIm);
#endif /* COMPLEX_FIELD_VALUES */
  }

  // Write image to file.
  switch (dump_type)
  {
    case CURRENT:
    {
      std::string cur_bmp_re = cur + std::string ("-Re") + std::string (".bmp");
      imageRe.WriteToFile (cur_bmp_re.c_str ());

#ifdef COMPLEX_FIELD_VALUES
      std::string cur_bmp_im = cur + std::string ("-Im") + std::string (".bmp");
      imageIm.WriteToFile (cur_bmp_im.c_str ());
#endif /* COMPLEX_FIELD_VALUES */

      break;
    }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
    case PREVIOUS:
    {
      std::string prev_bmp_re = cur + std::string ("-Re") + std::string (".bmp");
      imageRe.WriteToFile (prev_bmp_re.c_str ());

#ifdef COMPLEX_FIELD_VALUES
      std::string prev_bmp_im = cur + std::string ("-Im") + std::string (".bmp");
      imageIm.WriteToFile (prev_bmp_im.c_str ());
#endif /* COMPLEX_FIELD_VALUES */

      break;
    }
#if defined (TWO_TIME_STEPS)
    case PREVIOUS2:
    {
      std::string prevPrev_bmp_re = cur + std::string ("-Re") + std::string (".bmp");
      imageRe.WriteToFile (prevPrev_bmp_re.c_str ());

#ifdef COMPLEX_FIELD_VALUES
      std::string prevPrev_bmp_im = cur + std::string ("-Im") + std::string (".bmp");
      imageIm.WriteToFile (prevPrev_bmp_im.c_str ());
#endif /* COMPLEX_FIELD_VALUES */

      break;
    }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
    default:
    {
      UNREACHABLE;
    }
  }
}

/**
 * Save grid to file for specific layer for 2D.
 */
template<>
void
BMPDumper<GridCoordinate2D>::writeToFile (Grid<GridCoordinate2D> &grid, GridFileType dump_type) const
{
  const GridCoordinate2D& size = grid.getSize ();
  grid_coord sx = size.getX ();
  grid_coord sy = size.getY ();;

  // Create image for current values and max/min values.
  BMP imageRe;
  imageRe.SetSize (sx, sy);
  imageRe.SetBitDepth (24);

#ifdef COMPLEX_FIELD_VALUES
  BMP imageIm;
  imageIm.SetSize (sx, sy);
  imageIm.SetBitDepth (24);
#endif /* COMPLEX_FIELD_VALUES */

  const FieldPointValue* value0 = grid.getFieldPointValue (0);
  ASSERT (value0);

  FPValue maxPosRe = 0;
  FPValue maxNegRe = 0;

#ifdef COMPLEX_FIELD_VALUES
  FPValue maxPosIm = 0;
  FPValue maxNegIm = 0;
#endif /* COMPLEX_FIELD_VALUES */

  switch (dump_type)
  {
    case CURRENT:
    {
#ifdef COMPLEX_FIELD_VALUES
      maxNegRe = maxPosRe = value0->getCurValue ().real ();
      maxNegIm = maxPosIm = value0->getCurValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
      maxNegRe = maxPosRe = value0->getCurValue ();
#endif /* !COMPLEX_FIELD_VALUES */

      break;
    }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
    case PREVIOUS:
    {
#ifdef COMPLEX_FIELD_VALUES
      maxNegRe = maxPosRe = value0->getPrevValue ().real ();
      maxNegIm = maxPosIm = value0->getPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
      maxNegRe = maxPosRe = value0->getPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

      break;
    }
#if defined (TWO_TIME_STEPS)
    case PREVIOUS2:
    {
#ifdef COMPLEX_FIELD_VALUES
      maxNegRe = maxPosRe = value0->getPrevPrevValue ().real ();
      maxNegIm = maxPosIm = value0->getPrevPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
      maxNegRe = maxPosRe = value0->getPrevPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

      break;
    }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
    default:
    {
      UNREACHABLE;
    }
  }

  // Go through all values and calculate max/min.
  for (grid_iter i = 0; i < grid.getSize ().calculateTotalCoord (); ++i)
  {
    const FieldPointValue* current = grid.getFieldPointValue (i);

    ASSERT (current);

    FPValue valueRe = 0;

#ifdef COMPLEX_FIELD_VALUES
    FPValue valueIm = 0;
#endif /* COMPLEX_FIELD_VALUES */

    switch (dump_type)
    {
      case CURRENT:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getCurValue ().real ();
        valueIm = current->getCurValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getCurValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
      case PREVIOUS:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getPrevValue ().real ();
        valueIm = current->getPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (TWO_TIME_STEPS)
      case PREVIOUS2:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getPrevPrevValue ().real ();
        valueIm = current->getPrevPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getPrevPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
      default:
      {
        UNREACHABLE;
      }
    }

    if (valueRe > maxPosRe)
    {
      maxPosRe = valueRe;
    }
    if (valueRe < maxNegRe)
    {
      maxNegRe = valueRe;
    }

#ifdef COMPLEX_FIELD_VALUES
    if (valueIm > maxPosIm)
    {
      maxPosIm = valueIm;
    }
    if (valueIm < maxNegIm)
    {
      maxNegIm = valueIm;
    }
#endif /* COMPLEX_FIELD_VALUES */
  }

  // Set max (diff between max positive and max negative).
  const FPValue maxRe = maxPosRe - maxNegRe;

#ifdef COMPLEX_FIELD_VALUES
  const FPValue maxIm = maxPosIm - maxNegIm;
#endif /* COMPLEX_FIELD_VALUES */

  // Go through all values and set pixels.
  grid_iter end = grid.getSize().calculateTotalCoord ();
  for (grid_iter iter = 0; iter < end; ++iter)
  {
    // Get current point value.
    const FieldPointValue* current = grid.getFieldPointValue (iter);
    ASSERT (current);

    // Calculate its position from index in array.
    GridCoordinate2D coord = grid.calculatePositionFromIndex (iter);

    // Pixel coordinate.
    grid_iter px = coord.getX ();
    grid_iter py = coord.getY ();;

    // Get pixel for image.
    FPValue valueRe = 0;

#ifdef COMPLEX_FIELD_VALUES
    FPValue valueIm = 0;
#endif /* COMPLEX_FIELD_VALUES */

    switch (dump_type)
    {
      case CURRENT:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getCurValue ().real ();
        valueIm = current->getCurValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getCurValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
      case PREVIOUS:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getPrevValue ().real ();
        valueIm = current->getPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (TWO_TIME_STEPS)
      case PREVIOUS2:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getPrevPrevValue ().real ();
        valueIm = current->getPrevPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getPrevPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
      default:
      {
        UNREACHABLE;
      }
    }

    RGBApixel pixelRe = BMPhelper.getPixelFromValue (valueRe, maxNegRe, maxRe);

#ifdef COMPLEX_FIELD_VALUES
    RGBApixel pixelIm = BMPhelper.getPixelFromValue (valueIm, maxNegIm, maxIm);
#endif /* COMPLEX_FIELD_VALUES */

    // Set pixel for current image.
    imageRe.SetPixel(px, py, pixelRe);

#ifdef COMPLEX_FIELD_VALUES
    imageIm.SetPixel(px, py, pixelIm);
#endif /* COMPLEX_FIELD_VALUES */
  }

  // Write image to file.
  switch (dump_type)
  {
    case CURRENT:
    {
      std::string cur_bmp_re = cur + std::string ("-Re") + std::string (".bmp");
      imageRe.WriteToFile (cur_bmp_re.c_str ());

#ifdef COMPLEX_FIELD_VALUES
      std::string cur_bmp_im = cur + std::string ("-Im") + std::string (".bmp");
      imageIm.WriteToFile (cur_bmp_im.c_str ());
#endif /* COMPLEX_FIELD_VALUES */

      break;
    }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
    case PREVIOUS:
    {
      std::string prev_bmp_re = cur + std::string ("-Re") + std::string (".bmp");
      imageRe.WriteToFile (prev_bmp_re.c_str ());

#ifdef COMPLEX_FIELD_VALUES
      std::string prev_bmp_im = cur + std::string ("-Im") + std::string (".bmp");
      imageIm.WriteToFile (prev_bmp_im.c_str ());
#endif /* COMPLEX_FIELD_VALUES */

      break;
    }
#if defined (TWO_TIME_STEPS)
    case PREVIOUS2:
    {
      std::string prevPrev_bmp_re = cur + std::string ("-Re") + std::string (".bmp");
      imageRe.WriteToFile (prevPrev_bmp_re.c_str ());

#ifdef COMPLEX_FIELD_VALUES
      std::string prevPrev_bmp_im = cur + std::string ("-Im") + std::string (".bmp");
      imageIm.WriteToFile (prevPrev_bmp_im.c_str ());
#endif /* COMPLEX_FIELD_VALUES */

      break;
    }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
    default:
    {
      UNREACHABLE;
    }
  }
}

/**
 * Save grid to file for specific layer for 3D.
 */
template<>
void
BMPDumper<GridCoordinate3D>::writeToFile (Grid<GridCoordinate3D> &grid, GridFileType dump_type) const
{
//  ASSERT_MESSAGE ("3D Dumper is not implemented.")

  const GridCoordinate3D& size = grid.getSize ();
  grid_coord sx = size.getX ();
  grid_coord sy = size.getY ();
  grid_coord sz = size.getZ ();

  const FieldPointValue* value0 = grid.getFieldPointValue (0);
  ASSERT (value0);

  FPValue maxPosRe = 0;
  FPValue maxNegRe = 0;

#ifdef COMPLEX_FIELD_VALUES
  FPValue maxPosIm = 0;
  FPValue maxNegIm = 0;
#endif /* COMPLEX_FIELD_VALUES */

  switch (dump_type)
  {
    case CURRENT:
    {
#ifdef COMPLEX_FIELD_VALUES
      maxNegRe = maxPosRe = value0->getCurValue ().real ();
      maxNegIm = maxPosIm = value0->getCurValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
      maxNegRe = maxPosRe = value0->getCurValue ();
#endif /* !COMPLEX_FIELD_VALUES */

      break;
    }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
    case PREVIOUS:
    {
#ifdef COMPLEX_FIELD_VALUES
      maxNegRe = maxPosRe = value0->getPrevValue ().real ();
      maxNegIm = maxPosIm = value0->getPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
      maxNegRe = maxPosRe = value0->getPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

      break;
    }
#if defined (TWO_TIME_STEPS)
    case PREVIOUS2:
    {
#ifdef COMPLEX_FIELD_VALUES
      maxNegRe = maxPosRe = value0->getPrevPrevValue ().real ();
      maxNegIm = maxPosIm = value0->getPrevPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
      maxNegRe = maxPosRe = value0->getPrevPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

      break;
    }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
    default:
    {
      UNREACHABLE;
    }
  }

  // Go through all values and calculate max/min.
  for (grid_iter i = 0; i < grid.getSize ().calculateTotalCoord (); ++i)
  {
    const FieldPointValue* current = grid.getFieldPointValue (i);

    ASSERT (current);

    FPValue valueRe = 0;

#ifdef COMPLEX_FIELD_VALUES
    FPValue valueIm = 0;
#endif /* COMPLEX_FIELD_VALUES */

    switch (dump_type)
    {
      case CURRENT:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getCurValue ().real ();
        valueIm = current->getCurValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getCurValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
      case PREVIOUS:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getPrevValue ().real ();
        valueIm = current->getPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (TWO_TIME_STEPS)
      case PREVIOUS2:
      {
#ifdef COMPLEX_FIELD_VALUES
        valueRe = current->getPrevPrevValue ().real ();
        valueIm = current->getPrevPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
        valueRe = current->getPrevPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

        break;
      }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
      default:
      {
        UNREACHABLE;
      }
    }

    if (valueRe > maxPosRe)
    {
      maxPosRe = valueRe;
    }
    if (valueRe < maxNegRe)
    {
      maxNegRe = valueRe;
    }

#ifdef COMPLEX_FIELD_VALUES
    if (valueIm > maxPosIm)
    {
      maxPosIm = valueIm;
    }
    if (valueIm < maxNegIm)
    {
      maxNegIm = valueIm;
    }
#endif /* COMPLEX_FIELD_VALUES */
  }

  // Set max (diff between max positive and max negative).
  const FPValue maxRe = maxPosRe - maxNegRe;

#ifdef COMPLEX_FIELD_VALUES
  const FPValue maxIm = maxPosIm - maxNegIm;
#endif /* COMPLEX_FIELD_VALUES */

  // Create image for current values and max/min values.
  for (grid_coord k = 0; k < sz; ++k)
  {
    BMP imageRe;
    imageRe.SetSize (sx, sy);
    imageRe.SetBitDepth (24);

#ifdef COMPLEX_FIELD_VALUES
    BMP imageIm;
    imageIm.SetSize (sx, sy);
    imageIm.SetBitDepth (24);
#endif /* COMPLEX_FIELD_VALUES */

    for (grid_iter i = 0; i < size.getX (); ++i)
    {
      for (grid_iter j = 0; j < size.getY (); ++j)
      {
        GridCoordinate3D pos (i, j, k);

        // Get current point value.
        const FieldPointValue* current = grid.getFieldPointValue (pos);
        ASSERT (current);

        // Get pixel for image.
        FPValue valueRe = 0;

#ifdef COMPLEX_FIELD_VALUES
        FPValue valueIm = 0;
#endif /* COMPLEX_FIELD_VALUES */

        switch (dump_type)
        {
          case CURRENT:
          {
#ifdef COMPLEX_FIELD_VALUES
            valueRe = current->getCurValue ().real ();
            valueIm = current->getCurValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
            valueRe = current->getCurValue ();
#endif /* !COMPLEX_FIELD_VALUES */

            break;
          }
  #if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
          case PREVIOUS:
          {
#ifdef COMPLEX_FIELD_VALUES
            valueRe = current->getPrevValue ().real ();
            valueIm = current->getPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
            valueRe = current->getPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

            break;
          }
  #if defined (TWO_TIME_STEPS)
          case PREVIOUS2:
          {
#ifdef COMPLEX_FIELD_VALUES
            valueRe = current->getPrevPrevValue ().real ();
            valueIm = current->getPrevPrevValue ().imag ();
#else /* COMPLEX_FIELD_VALUES */
            valueRe = current->getPrevPrevValue ();
#endif /* !COMPLEX_FIELD_VALUES */

            break;
          }
  #endif /* TWO_TIME_STEPS */
  #endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
          default:
          {
            UNREACHABLE;
          }
        }

        RGBApixel pixelRe = BMPhelper.getPixelFromValue (valueRe, maxNegRe, maxRe);

#ifdef COMPLEX_FIELD_VALUES
        RGBApixel pixelIm = BMPhelper.getPixelFromValue (valueIm, maxNegIm, maxIm);
#endif /* COMPLEX_FIELD_VALUES */

        // Set pixel for current image.
        imageRe.SetPixel(i, j, pixelRe);

#ifdef COMPLEX_FIELD_VALUES
        imageIm.SetPixel(i, j, pixelIm);
#endif /* COMPLEX_FIELD_VALUES */
      }
    }

    // Write image to file.
    switch (dump_type)
    {
      case CURRENT:
      {
        std::string cur_bmp_re = cur + int64_to_string (k) + std::string ("-Re") + std::string (".bmp");
        imageRe.WriteToFile (cur_bmp_re.c_str ());

#ifdef COMPLEX_FIELD_VALUES
        std::string cur_bmp_im = cur + int64_to_string (k) + std::string ("-Im") + std::string (".bmp");
        imageIm.WriteToFile (cur_bmp_im.c_str ());
#endif /* COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
      case PREVIOUS:
      {
        std::string prev_bmp_re = cur + int64_to_string (k) + std::string ("-Re") + std::string (".bmp");
        imageRe.WriteToFile (prev_bmp_re.c_str ());

#ifdef COMPLEX_FIELD_VALUES
        std::string prev_bmp_im = cur + int64_to_string (k) + std::string ("-Im") + std::string (".bmp");
        imageIm.WriteToFile (prev_bmp_im.c_str ());
#endif /* COMPLEX_FIELD_VALUES */

        break;
      }
#if defined (TWO_TIME_STEPS)
      case PREVIOUS2:
      {
        std::string prevPrev_bmp_re = cur + int64_to_string (k) + std::string ("-Re") + std::string (".bmp");
        imageRe.WriteToFile (prevPrev_bmp_re.c_str ());

#ifdef COMPLEX_FIELD_VALUES
        std::string prevPrev_bmp_im = cur + int64_to_string (k) + std::string ("-Im") + std::string (".bmp");
        imageIm.WriteToFile (prevPrev_bmp_im.c_str ());
#endif /* COMPLEX_FIELD_VALUES */

        break;
      }
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */
      default:
      {
        UNREACHABLE;
      }
    }
  }
}
