import {
  IsString,
  IsEnum,
  IsNumber,
  IsOptional,
  IsDateString,
  Min,
  Max,
} from 'class-validator';
import { GradeType, Semester } from '../entities/grade.entity';

export class CreateGradeDto {
  @IsString()
  studentId: string;

  @IsString()
  subjectCode: string;

  @IsString()
  subjectName: string;

  @IsEnum(GradeType)
  type: GradeType;

  @IsNumber()
  @Min(0)
  score: number;

  @IsNumber()
  @Min(0)
  @IsOptional()
  maxScore?: number;

  @IsNumber()
  @Min(0)
  @Max(1)
  @IsOptional()
  weight?: number;

  @IsString()
  @IsOptional()
  letterGrade?: string;

  @IsString()
  academicYear: string;

  @IsEnum(Semester)
  semester: Semester;

  @IsString()
  @IsOptional()
  comments?: string;

  @IsString()
  @IsOptional()
  gradedBy?: string;

  @IsDateString()
  gradeDate: string;
}
