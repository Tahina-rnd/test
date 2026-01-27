import {
  IsString,
  IsEnum,
  IsOptional,
  IsDateString,
  IsNumber,
  Min,
  Max,
} from 'class-validator';
import { AcademicLevel, StudentStatus } from '../entities/student.entity';

export class CreateStudentDto {
  @IsString()
  userId: string;

  @IsString()
  studentNumber: string;

  @IsEnum(AcademicLevel)
  academicLevel: AcademicLevel;

  @IsEnum(StudentStatus)
  @IsOptional()
  status?: StudentStatus;

  @IsDateString()
  enrollmentDate: string;

  @IsString()
  @IsOptional()
  specialization?: string;

  @IsNumber()
  @Min(0)
  @Max(4)
  @IsOptional()
  gpa?: number;

  @IsNumber()
  @Min(0)
  @IsOptional()
  totalCredits?: number;

  @IsString()
  @IsOptional()
  emergencyContactName?: string;

  @IsString()
  @IsOptional()
  emergencyContactPhone?: string;
}
